#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_core.h"
// TODO: This is used for printing the stack trace on an error. This should be
// behind a flag so that you can use Wren with all debugging info stripped out.
#include "wren_debug.h"
#include "wren_vm.h"

#if WREN_TRACE_MEMORY || WREN_TRACE_GC
#include <time.h>
#endif

// The built-in reallocation function used when one is not provided by the
// configuration.
static void* defaultReallocate(void* memory, size_t oldSize, size_t newSize)
{
  return realloc(memory, newSize);
}

WrenVM* wrenNewVM(WrenConfiguration* configuration)
{
  WrenReallocateFn reallocate = defaultReallocate;
  if (configuration->reallocateFn != NULL)
  {
    reallocate = configuration->reallocateFn;
  }

  WrenVM* vm = reallocate(NULL, 0, sizeof(WrenVM));

  vm->reallocate = reallocate;
  
  wrenSymbolTableInit(&vm->methods);
  wrenSymbolTableInit(&vm->globalSymbols);

  vm->bytesAllocated = 0;

  vm->nextGC = 1024 * 1024 * 10;
  if (configuration->initialHeapSize != 0)
  {
    vm->nextGC = configuration->initialHeapSize;
  }

  vm->minNextGC = 1024 * 1024;
  if (configuration->minHeapSize != 0)
  {
    vm->minNextGC = configuration->minHeapSize;
  }

  vm->heapScalePercent = 150;
  if (configuration->heapGrowthPercent != 0)
  {
    // +100 here because the configuration gives us the *additional* size of
    // the heap relative to the in-use memory, while heapScalePercent is the
    // *total* size of the heap relative to in-use.
    vm->heapScalePercent = 100 + configuration->heapGrowthPercent;
  }

  vm->compiler = NULL;
  vm->first = NULL;
  vm->pinned = NULL;

  // Clear out the global variables. This ensures they are NULL before being
  // initialized in case we do a garbage collection before one gets initialized.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    vm->globals[i] = NULL_VAL;
  }

  vm->foreignCallSlot = NULL;
  vm->foreignCallNumArgs = 0;

  wrenInitializeCore(vm);
  return vm;
}

void wrenFreeVM(WrenVM* vm)
{
  wrenSymbolTableClear(vm, &vm->methods);
  wrenSymbolTableClear(vm, &vm->globalSymbols);
  wrenReallocate(vm, vm, 0, 0);
}

static void collectGarbage(WrenVM* vm);

static void markValue(WrenVM* vm, Value value);

static void markClass(WrenVM* vm, ObjClass* classObj)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (classObj->obj.flags & FLAG_MARKED) return;
  classObj->obj.flags |= FLAG_MARKED;

  // The metaclass.
  if (classObj->metaclass != NULL) markClass(vm, classObj->metaclass);

  // The superclass.
  if (classObj->superclass != NULL) markClass(vm, classObj->superclass);

  // Method function objects.
  for (int i = 0; i < MAX_SYMBOLS; i++)
  {
    if (classObj->methods[i].type == METHOD_BLOCK)
    {
      wrenMarkObj(vm, classObj->methods[i].fn);
    }
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjClass);
}

static void markFn(WrenVM* vm, ObjFn* fn)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (fn->obj.flags & FLAG_MARKED) return;
  fn->obj.flags |= FLAG_MARKED;

  // Mark the constants.
  for (int i = 0; i < fn->numConstants; i++)
  {
    markValue(vm, fn->constants[i]);
  }

  wrenMarkObj(vm, (Obj*)fn->debug->sourcePath);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjFn);
  vm->bytesAllocated += sizeof(uint8_t) * fn->bytecodeLength;
  vm->bytesAllocated += sizeof(Value) * fn->numConstants;

  // The debug line number buffer.
  vm->bytesAllocated += sizeof(int) * fn->bytecodeLength;

  // TODO: What about the function name?
}

static void markInstance(WrenVM* vm, ObjInstance* instance)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (instance->obj.flags & FLAG_MARKED) return;
  instance->obj.flags |= FLAG_MARKED;

  markClass(vm, instance->classObj);

  // Mark the fields.
  for (int i = 0; i < instance->classObj->numFields; i++)
  {
    markValue(vm, instance->fields[i]);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjInstance);
  vm->bytesAllocated += sizeof(Value) * instance->classObj->numFields;
}

static void markList(WrenVM* vm, ObjList* list)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (list->obj.flags & FLAG_MARKED) return;
  list->obj.flags |= FLAG_MARKED;

  // Mark the elements.
  Value* elements = list->elements;
  for (int i = 0; i < list->count; i++)
  {
    markValue(vm, elements[i]);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjList);
  if (list->elements != NULL)
  {
    vm->bytesAllocated += sizeof(Value) * list->capacity;
  }
}

static void markUpvalue(WrenVM* vm, Upvalue* upvalue)
{
  // This can happen if a GC is triggered in the middle of initializing the
  // closure.
  if (upvalue == NULL) return;

  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (upvalue->obj.flags & FLAG_MARKED) return;
  upvalue->obj.flags |= FLAG_MARKED;

  // Mark the closed-over object (in case it is closed).
  markValue(vm, upvalue->closed);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(Upvalue);
}

static void markFiber(WrenVM* vm, ObjFiber* fiber)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (fiber->obj.flags & FLAG_MARKED) return;
  fiber->obj.flags |= FLAG_MARKED;

  // Stack functions.
  for (int k = 0; k < fiber->numFrames; k++)
  {
    wrenMarkObj(vm, fiber->frames[k].fn);
  }

  // Stack variables.
  for (int l = 0; l < fiber->stackSize; l++)
  {
    markValue(vm, fiber->stack[l]);
  }

  // Open upvalues.
  Upvalue* upvalue = fiber->openUpvalues;
  while (upvalue != NULL)
  {
    markUpvalue(vm, upvalue);
    upvalue = upvalue->next;
  }
}

static void markClosure(WrenVM* vm, ObjClosure* closure)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (closure->obj.flags & FLAG_MARKED) return;
  closure->obj.flags |= FLAG_MARKED;

  // Mark the function.
  markFn(vm, closure->fn);

  // Mark the upvalues.
  for (int i = 0; i < closure->fn->numUpvalues; i++)
  {
    Upvalue** upvalues = closure->upvalues;
    Upvalue* upvalue = upvalues[i];
    markUpvalue(vm, upvalue);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjClosure);
  vm->bytesAllocated += sizeof(Upvalue*) * closure->fn->numUpvalues;
}

static void markString(WrenVM* vm, ObjString* string)
{
  // Don't recurse if already marked. Avoids getting stuck in a loop on cycles.
  if (string->obj.flags & FLAG_MARKED) return;
  string->obj.flags |= FLAG_MARKED;

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjString);
  // TODO: O(n) calculation here is lame!
  vm->bytesAllocated += strlen(string->value);
}

void wrenMarkObj(WrenVM* vm, Obj* obj)
{
#if WREN_TRACE_MEMORY
  static int indent = 0;
  indent++;
  for (int i = 0; i < indent; i++) printf("  ");
  printf("mark ");
  wrenPrintValue(OBJ_VAL(obj));
  printf(" @ %p\n", obj);
#endif

  // Traverse the object's fields.
  switch (obj->type)
  {
    case OBJ_CLASS:    markClass(   vm, (ObjClass*)   obj); break;
    case OBJ_CLOSURE:  markClosure( vm, (ObjClosure*) obj); break;
    case OBJ_FIBER:    markFiber(   vm, (ObjFiber*)   obj); break;
    case OBJ_FN:       markFn(      vm, (ObjFn*)      obj); break;
    case OBJ_INSTANCE: markInstance(vm, (ObjInstance*)obj); break;
    case OBJ_LIST:     markList(    vm, (ObjList*)    obj); break;
    case OBJ_STRING:   markString(  vm, (ObjString*)  obj); break;
    case OBJ_UPVALUE:  markUpvalue( vm, (Upvalue*)    obj); break;
  }

#if WREN_TRACE_MEMORY
  indent--;
#endif
}

void markValue(WrenVM* vm, Value value)
{
  if (!IS_OBJ(value)) return;
  wrenMarkObj(vm, AS_OBJ(value));
}

void wrenSetCompiler(WrenVM* vm, Compiler* compiler)
{
  vm->compiler = compiler;
}

static void collectGarbage(WrenVM* vm)
{
#if WREN_TRACE_MEMORY || WREN_TRACE_GC
  printf("-- gc --\n");

  size_t before = vm->bytesAllocated;
  double startTime = (double)clock() / CLOCKS_PER_SEC;
#endif

  // Mark all reachable objects.

  // Reset this. As we mark objects, their size will be counted again so that
  // we can track how much memory is in use without needing to know the size
  // of each *freed* object.
  //
  // This is important because when freeing an unmarked object, we don't always
  // know how much memory it is using. For example, when freeing an instance,
  // we need to know its class to know how big it is, but it's class may have
  // already been freed.
  vm->bytesAllocated = 0;

  // Global variables.
  for (int i = 0; i < vm->globalSymbols.count; i++)
  {
    // Check for NULL to handle globals that have been defined (at compile time)
    // but not yet initialized.
    if (!IS_NULL(vm->globals[i])) markValue(vm, vm->globals[i]);
  }

  // Pinned objects.
  PinnedObj* pinned = vm->pinned;
  while (pinned != NULL)
  {
    wrenMarkObj(vm, pinned->obj);
    pinned = pinned->previous;
  }

  // Any object the compiler is using (if there is one).
  if (vm->compiler != NULL) wrenMarkCompiler(vm, vm->compiler);

  // Collect any unmarked objects.
  Obj** obj = &vm->first;
  while (*obj != NULL)
  {
    if (!((*obj)->flags & FLAG_MARKED))
    {
      // This object wasn't reached, so remove it from the list and free it.
      Obj* unreached = *obj;
      *obj = unreached->next;
      wrenFreeObj(vm, unreached);
    }
    else
    {
      // This object was reached, so unmark it (for the next GC) and move on to
      // the next.
      (*obj)->flags &= ~FLAG_MARKED;
      obj = &(*obj)->next;
    }
  }

  vm->nextGC = vm->bytesAllocated * vm->heapScalePercent / 100;
  if (vm->nextGC < vm->minNextGC) vm->nextGC = vm->minNextGC;

#if WREN_TRACE_MEMORY || WREN_TRACE_GC
  double elapsed = ((double)clock() / CLOCKS_PER_SEC) - startTime;
  printf("GC %ld before, %ld after (%ld collected), next at %ld. Took %.3fs.\n",
         before, vm->bytesAllocated, before - vm->bytesAllocated, vm->nextGC,
         elapsed);
#endif
}

void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize)
{
#if WREN_TRACE_MEMORY
  printf("reallocate %p %ld -> %ld\n", memory, oldSize, newSize);
#endif

  // If new bytes are being allocated, add them to the total count. If objects
  // are being completely deallocated, we don't track that (since we don't
  // track the original size). Instead, that will be handled while marking
  // during the next GC.
  vm->bytesAllocated += newSize - oldSize;

#if WREN_DEBUG_GC_STRESS
  // Since collecting calls this function to free things, make sure we don't
  // recurse.
  if (newSize > 0) collectGarbage(vm);
#else
  if (vm->bytesAllocated > vm->nextGC) collectGarbage(vm);
#endif
  
  return vm->reallocate(memory, oldSize, newSize);
}

// Captures the local variable in [slot] into an [Upvalue]. If that local is
// already in an upvalue, the existing one will be used. (This is important to
// ensure that multiple closures closing over the same variable actually see
// the same variable.) Otherwise, it will create a new open upvalue and add it
// the fiber's list of upvalues.
static Upvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, int slot)
{
  Value* local = &fiber->stack[slot];

  // If there are no open upvalues at all, we must need a new one.
  if (fiber->openUpvalues == NULL)
  {
    fiber->openUpvalues = wrenNewUpvalue(vm, local);
    return fiber->openUpvalues;
  }

  Upvalue* prevUpvalue = NULL;
  Upvalue* upvalue = fiber->openUpvalues;

  // Walk towards the bottom of the stack until we find a previously existsing
  // upvalue or pass where it should be.
  while (upvalue != NULL && upvalue->value > local)
  {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  // Found an existing upvalue for this local.
  if (upvalue->value == local) return upvalue;

  // We've walked past this local on the stack, so there must not be an
  // upvalue for it already. Make a new one and link it in in the right
  // place to keep the list sorted.
  Upvalue* createdUpvalue = wrenNewUpvalue(vm, local);
  if (prevUpvalue == NULL)
  {
    // The new one is the first one in the list.
    fiber->openUpvalues = createdUpvalue;
  }
  else
  {
    prevUpvalue->next = createdUpvalue;
  }

  createdUpvalue->next = upvalue;
  return createdUpvalue;
}

static void closeUpvalue(ObjFiber* fiber)
{
  Upvalue* upvalue = fiber->openUpvalues;

  // Move the value into the upvalue itself and point the upvalue to it.
  upvalue->closed = *upvalue->value;
  upvalue->value = &upvalue->closed;

  // Remove it from the open upvalue list.
  fiber->openUpvalues = upvalue->next;
}

static void bindMethod(int methodType, int symbol, ObjClass* classObj,
                       Value method)
{
  ObjFn* methodFn = IS_FN(method) ? AS_FN(method) : AS_CLOSURE(method)->fn;

  // Methods are always bound against the class, and not the metaclass, even
  // for static methods, so that constructors (which are static) get bound like
  // instance methods.
  wrenBindMethod(classObj, methodFn);

  // TODO: Note that this code could be simplified, but doing so seems to
  // degrade performance on the method_call benchmark. My guess is simplifying
  // this causes this function to be small enough to be inlined in the bytecode
  // loop, which then affects instruction caching.
  switch (methodType)
  {
    case CODE_METHOD_INSTANCE:
      classObj->methods[symbol].type = METHOD_BLOCK;
      classObj->methods[symbol].fn = AS_OBJ(method);
      break;

    case CODE_METHOD_STATIC:
      // Statics are defined on the metaclass.
      classObj->metaclass->methods[symbol].type = METHOD_BLOCK;
      classObj->metaclass->methods[symbol].fn = AS_OBJ(method);
      break;
  }
}

static void callForeign(WrenVM* vm, ObjFiber* fiber,
                        WrenForeignMethodFn foreign, int numArgs)
{
  vm->foreignCallSlot = &fiber->stack[fiber->stackSize - numArgs];

  // Don't include the receiver.
  vm->foreignCallNumArgs = numArgs - 1;

  foreign(vm);

  // Discard the stack slots for the arguments (but leave one for
  // the result).
  fiber->stackSize -= numArgs - 1;

  // If nothing was returned, implicitly return null.
  if (vm->foreignCallSlot != NULL)
  {
    *vm->foreignCallSlot = NULL_VAL;
    vm->foreignCallSlot = NULL;
  }
}

static void methodNotFound(WrenVM* vm, ObjFiber* fiber, Value* receiver,
                           int symbol)
{
  // TODO: Tune size.
  char message[200];

  // TODO: Include receiver in message.
  snprintf(message, 200, "Receiver does not implement method '%s'.",
           vm->methods.names[symbol]);

  // Store the error message in the receiver slot so that it's on the fiber's
  // stack and doesn't get garbage collected.
  *receiver = wrenNewString(vm, message, strlen(message));

  wrenDebugPrintStackTrace(vm, fiber, *receiver);
}

// Pushes [function] onto [fiber]'s callstack and invokes it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
// [function] can be an `ObjFn` or `ObjClosure`.
static void callFunction(ObjFiber* fiber, Obj* function, int numArgs)
{
  // TODO: Check for stack overflow.
  CallFrame* frame = &fiber->frames[fiber->numFrames];
  frame->fn = function;
  frame->stackStart = fiber->stackSize - numArgs;

  frame->ip = 0;
  if (function->type == OBJ_FN)
  {
    frame->ip = ((ObjFn*)function)->bytecode;
  }
  else
  {
    frame->ip = ((ObjClosure*)function)->fn->bytecode;
  }

  fiber->numFrames++;
}

// The main bytecode interpreter loop. This is where the magic happens. It is
// also, as you can imagine, highly performance critical. Returns `true` if the
// fiber completed without error.
static bool interpret(WrenVM* vm, ObjFiber* fiber)
{
  // These macros are designed to only be invoked within this function.
  // TODO: Check for stack overflow.
  #define PUSH(value) (fiber->stack[fiber->stackSize++] = value)
  #define POP()       (fiber->stack[--fiber->stackSize])
  #define PEEK()      (fiber->stack[fiber->stackSize - 1])
  #define READ_ARG()  (*ip++)

  // Hoist these into local variables. They are accessed frequently in the loop
  // but assigned less frequently. Keeping them in locals and updating them when
  // a call frame has been pushed or popped gives a large speed boost.
  register CallFrame* frame;
  register uint8_t* ip;
  register ObjFn* fn;
  register Upvalue** upvalues;

  // Use this before a CallFrame is pushed to store the local variables back
  // into the current one.
  #define STORE_FRAME() frame->ip = ip

  // Use this after a CallFrame has been pushed or popped to refresh the local
  // variables.
  #define LOAD_FRAME()                                 \
      frame = &fiber->frames[fiber->numFrames - 1];    \
      ip = frame->ip;                                  \
      if (frame->fn->type == OBJ_FN)                   \
      {                                                \
        fn = (ObjFn*)frame->fn;                        \
        upvalues = NULL;                               \
      }                                                \
      else                                             \
      {                                                \
        fn = ((ObjClosure*)frame->fn)->fn;             \
        upvalues = ((ObjClosure*)frame->fn)->upvalues; \
      }

  #if WREN_COMPUTED_GOTO

  // Note that the order of instructions here must exacly match the Code enum
  // in wren_vm.h or horrendously bad things happen.
  static void* dispatchTable[] = {
    &&code_CONSTANT,
    &&code_NULL,
    &&code_FALSE,
    &&code_TRUE,
    &&code_LOAD_LOCAL,
    &&code_STORE_LOCAL,
    &&code_LOAD_UPVALUE,
    &&code_STORE_UPVALUE,
    &&code_LOAD_GLOBAL,
    &&code_STORE_GLOBAL,
    &&code_LOAD_FIELD_THIS,
    &&code_STORE_FIELD_THIS,
    &&code_LOAD_FIELD,
    &&code_STORE_FIELD,
    &&code_POP,
    &&code_CALL_0,
    &&code_CALL_1,
    &&code_CALL_2,
    &&code_CALL_3,
    &&code_CALL_4,
    &&code_CALL_5,
    &&code_CALL_6,
    &&code_CALL_7,
    &&code_CALL_8,
    &&code_CALL_9,
    &&code_CALL_10,
    &&code_CALL_11,
    &&code_CALL_12,
    &&code_CALL_13,
    &&code_CALL_14,
    &&code_CALL_15,
    &&code_CALL_16,
    &&code_SUPER_0,
    &&code_SUPER_1,
    &&code_SUPER_2,
    &&code_SUPER_3,
    &&code_SUPER_4,
    &&code_SUPER_5,
    &&code_SUPER_6,
    &&code_SUPER_7,
    &&code_SUPER_8,
    &&code_SUPER_9,
    &&code_SUPER_10,
    &&code_SUPER_11,
    &&code_SUPER_12,
    &&code_SUPER_13,
    &&code_SUPER_14,
    &&code_SUPER_15,
    &&code_SUPER_16,
    &&code_JUMP,
    &&code_LOOP,
    &&code_JUMP_IF,
    &&code_AND,
    &&code_OR,
    &&code_IS,
    &&code_CLOSE_UPVALUE,
    &&code_RETURN,
    &&code_NEW,
    &&code_LIST,
    &&code_CLOSURE,
    &&code_CLASS,
    &&code_SUBCLASS,
    &&code_METHOD_INSTANCE,
    &&code_METHOD_STATIC
  };

  #define INTERPRET_LOOP    DISPATCH();
  #define CASE_CODE(name)   code_##name
  #define DISPATCH()        goto *dispatchTable[instruction = *ip++]

  // If you want to debug the VM and see the stack as each instruction is
  // executed, uncomment this.
  // TODO: Use a #define to enable/disable this.
  /*
  #define DISPATCH() \
    { \
      wrenDebugPrintStack(fiber); \
      wrenDebugPrintInstruction(vm, fn, (int)(ip - fn->bytecode)); \
      instruction = *ip++; \
      goto *dispatchTable[instruction]; \
    }
  */

  #else

  #define INTERPRET_LOOP    for (;;) switch (instruction = *ip++)
  #define CASE_CODE(name)   case CODE_##name
  #define DISPATCH()        break

  #endif

  LOAD_FRAME();

  Code instruction;
  INTERPRET_LOOP
  {
    CASE_CODE(POP): POP(); DISPATCH();

    CASE_CODE(NULL):  PUSH(NULL_VAL); DISPATCH();
    CASE_CODE(FALSE): PUSH(FALSE_VAL); DISPATCH();
    CASE_CODE(TRUE):  PUSH(TRUE_VAL); DISPATCH();

    CASE_CODE(CALL_0):
    CASE_CODE(CALL_1):
    CASE_CODE(CALL_2):
    CASE_CODE(CALL_3):
    CASE_CODE(CALL_4):
    CASE_CODE(CALL_5):
    CASE_CODE(CALL_6):
    CASE_CODE(CALL_7):
    CASE_CODE(CALL_8):
    CASE_CODE(CALL_9):
    CASE_CODE(CALL_10):
    CASE_CODE(CALL_11):
    CASE_CODE(CALL_12):
    CASE_CODE(CALL_13):
    CASE_CODE(CALL_14):
    CASE_CODE(CALL_15):
    CASE_CODE(CALL_16):
    {
      // Add one for the implicit receiver argument.
      int numArgs = instruction - CODE_CALL_0 + 1;
      int symbol = READ_ARG();

      Value receiver = fiber->stack[fiber->stackSize - numArgs];
      ObjClass* classObj = wrenGetClass(vm, receiver);
      Method* method = &classObj->methods[symbol];
      switch (method->type)
      {
        case METHOD_PRIMITIVE:
        {
          Value* args = &fiber->stack[fiber->stackSize - numArgs];

          // After calling this, the result will be in the first arg slot.
          switch (method->primitive(vm, fiber, args))
          {
            case PRIM_VALUE:
              // The result is now in the first arg slot. Discard the other
              // stack slots.
              fiber->stackSize -= numArgs - 1;
              break;

            case PRIM_ERROR:
              STORE_FRAME();
              wrenDebugPrintStackTrace(vm, fiber, args[0]);
              return false;

            case PRIM_CALL:
              STORE_FRAME();
              // TODO: What if the function doesn't expect the same number of
              // args?
              callFunction(fiber, AS_OBJ(args[0]), numArgs);
              LOAD_FRAME();
              break;
          }
          break;
        }

        case METHOD_FOREIGN:
          callForeign(vm, fiber, method->foreign, numArgs);
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(fiber, method->fn, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          STORE_FRAME();
          methodNotFound(vm, fiber, &fiber->stack[fiber->stackSize - numArgs],
                         symbol);
          return false;
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_LOCAL):
      PUSH(fiber->stack[frame->stackStart + READ_ARG()]);
      DISPATCH();

    CASE_CODE(STORE_LOCAL):
      fiber->stack[frame->stackStart + READ_ARG()] = PEEK();
      DISPATCH();

    CASE_CODE(CONSTANT):
      PUSH(fn->constants[READ_ARG()]);
      DISPATCH();

    CASE_CODE(SUPER_0):
    CASE_CODE(SUPER_1):
    CASE_CODE(SUPER_2):
    CASE_CODE(SUPER_3):
    CASE_CODE(SUPER_4):
    CASE_CODE(SUPER_5):
    CASE_CODE(SUPER_6):
    CASE_CODE(SUPER_7):
    CASE_CODE(SUPER_8):
    CASE_CODE(SUPER_9):
    CASE_CODE(SUPER_10):
    CASE_CODE(SUPER_11):
    CASE_CODE(SUPER_12):
    CASE_CODE(SUPER_13):
    CASE_CODE(SUPER_14):
    CASE_CODE(SUPER_15):
    CASE_CODE(SUPER_16):
    {
      // TODO: Almost completely copied from CALL. Unify somehow.

      // Add one for the implicit receiver argument.
      int numArgs = instruction - CODE_SUPER_0 + 1;
      int symbol = READ_ARG();

      Value receiver = fiber->stack[fiber->stackSize - numArgs];
      ObjClass* classObj = wrenGetClass(vm, receiver);

      // Ignore methods defined on the receiver's immediate class.
      classObj = classObj->superclass;

      Method* method = &classObj->methods[symbol];
      switch (method->type)
      {
        case METHOD_PRIMITIVE:
        {
          Value* args = &fiber->stack[fiber->stackSize - numArgs];

          // After calling this, the result will be in the first arg slot.
          switch (method->primitive(vm, fiber, args))
          {
            case PRIM_VALUE:
              // The result is now in the first arg slot. Discard the other
              // stack slots.
              fiber->stackSize -= numArgs - 1;
              break;

            case PRIM_ERROR:
              STORE_FRAME();
              wrenDebugPrintStackTrace(vm, fiber, args[0]);
              return false;

            case PRIM_CALL:
              STORE_FRAME();
              callFunction(fiber, AS_OBJ(args[0]), numArgs);
              LOAD_FRAME();
              break;
          }
          break;
        }

        case METHOD_FOREIGN:
          callForeign(vm, fiber, method->foreign, numArgs);
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(fiber, method->fn, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          STORE_FRAME();
          methodNotFound(vm, fiber, &fiber->stack[fiber->stackSize - numArgs],
                         symbol);
          return false;
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_UPVALUE):
      ASSERT(upvalues != NULL,
             "Should not have CODE_LOAD_UPVALUE instruction in non-closure.");
      PUSH(*upvalues[READ_ARG()]->value);
      DISPATCH();

    CASE_CODE(STORE_UPVALUE):
      ASSERT(upvalues != NULL,
             "Should not have CODE_STORE_UPVALUE instruction in non-closure.");
      *upvalues[READ_ARG()]->value = POP();
      DISPATCH();

    CASE_CODE(LOAD_GLOBAL):
      PUSH(vm->globals[READ_ARG()]);
      DISPATCH();

    CASE_CODE(STORE_GLOBAL):
      vm->globals[READ_ARG()] = PEEK();
      DISPATCH();

    CASE_CODE(LOAD_FIELD_THIS):
    {
      int field = READ_ARG();
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD_THIS):
    {
      int field = READ_ARG();
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_FIELD):
    {
      int field = READ_ARG();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD):
    {
      int field = READ_ARG();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(JUMP):
    {
      int offset = READ_ARG();
      ip += offset;
      DISPATCH();
    }

    CASE_CODE(LOOP):
    {
      // Jump back to the top of the loop.
      int offset = READ_ARG();
      ip -= offset;
      DISPATCH();
    }

    CASE_CODE(JUMP_IF):
    {
      int offset = READ_ARG();
      Value condition = POP();

      // False is the only falsey value.
      // TODO: Null should be falsey too.
      if (IS_FALSE(condition)) ip += offset;
      DISPATCH();
    }

    CASE_CODE(AND):
    {
      int offset = READ_ARG();
      Value condition = PEEK();

      // False is the only falsey value.
      // TODO: Null should be falsey too.
      if (!IS_FALSE(condition))
      {
        // Discard the condition and evaluate the right hand side.
        POP();
      }
      else
      {
        // Short-circuit the right hand side.
        ip += offset;
      }
      DISPATCH();
    }

    CASE_CODE(OR):
    {
      int offset = READ_ARG();
      Value condition = PEEK();

      // False is the only falsey value.
      // TODO: Null should be falsey too.
      if (IS_FALSE(condition))
      {
        // Discard the condition and evaluate the right hand side.
        POP();
      }
      else
      {
        // Short-circuit the right hand side.
        ip += offset;
      }
      DISPATCH();
    }

    CASE_CODE(IS):
    {
      // TODO: What if classObj is not a class?
      ObjClass* expected = AS_CLASS(POP());
      Value obj = POP();

      ObjClass* actual = wrenGetClass(vm, obj);
      bool isInstance = false;

      // Walk the superclass chain looking for the class.
      while (actual != NULL)
      {
        if (actual == expected)
        {
          isInstance = true;
          break;
        }
        actual = actual->superclass;
      }
      PUSH(BOOL_VAL(isInstance));
      DISPATCH();
    }

    CASE_CODE(CLOSE_UPVALUE):
      closeUpvalue(fiber);
      POP();
      DISPATCH();

    CASE_CODE(NEW):
    {
      // TODO: Handle object not being a class.
      ObjClass* classObj = AS_CLASS(POP());
      PUSH(wrenNewInstance(vm, classObj));
      DISPATCH();
    }

    CASE_CODE(RETURN):
    {
      Value result = POP();
      fiber->numFrames--;

      // If we are returning from the top-level block, we succeeded.
      if (fiber->numFrames == 0) return true;

      // Close any upvalues still in scope.
      Value* firstValue = &fiber->stack[frame->stackStart];
      while (fiber->openUpvalues != NULL &&
             fiber->openUpvalues->value >= firstValue)
      {
        closeUpvalue(fiber);
      }

      // Store the result of the block in the first slot, which is where the
      // caller expects it.
      fiber->stack[frame->stackStart] = result;

      // Discard the stack slots for the call frame (leaving one slot for the
      // result).
      fiber->stackSize = frame->stackStart + 1;
      LOAD_FRAME();
      DISPATCH();
    }

    CASE_CODE(LIST):
    {
      int numElements = READ_ARG();
      ObjList* list = wrenNewList(vm, numElements);
      for (int i = 0; i < numElements; i++)
      {
        list->elements[i] = fiber->stack[fiber->stackSize - numElements + i];
      }

      // Discard the elements.
      fiber->stackSize -= numElements;

      PUSH(OBJ_VAL(list));
      DISPATCH();
    }

    CASE_CODE(CLOSURE):
    {
      ObjFn* prototype = AS_FN(fn->constants[READ_ARG()]);

      ASSERT(prototype->numUpvalues > 0,
             "Should not create closure for functions that don't need it.");

      // Create the closure and push it on the stack before creating upvalues
      // so that it doesn't get collected.
      ObjClosure* closure = wrenNewClosure(vm, prototype);
      PUSH(OBJ_VAL(closure));

      // Capture upvalues.
      for (int i = 0; i < prototype->numUpvalues; i++)
      {
        bool isLocal = READ_ARG();
        int index = READ_ARG();
        if (isLocal)
        {
          // Make an new upvalue to close over the parent's local variable.
          closure->upvalues[i] = captureUpvalue(vm, fiber,
                                                frame->stackStart + index);
        }
        else
        {
          // Use the same upvalue as the current call frame.
          closure->upvalues[i] = upvalues[index];
        }
      }

      DISPATCH();
    }

    CASE_CODE(CLASS):
    CASE_CODE(SUBCLASS):
    {
      bool isSubclass = instruction == CODE_SUBCLASS;
      int numFields = READ_ARG();

      ObjClass* superclass;
      if (isSubclass)
      {
        // TODO: Handle the superclass not being a class object!
        superclass = AS_CLASS(POP());
      }
      else
      {
        // Implicit Object superclass.
        superclass = vm->objectClass;
      }

      ObjClass* classObj = wrenNewClass(vm, superclass, numFields);

      PUSH(OBJ_VAL(classObj));
      DISPATCH();
    }

    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      int type = instruction;
      int symbol = READ_ARG();
      Value method = POP();
      ObjClass* classObj = AS_CLASS(PEEK());
      bindMethod(type, symbol, classObj, method);
      DISPATCH();
    }

    CASE_CODE(END):
      // A CODE_END should always be preceded by a CODE_RETURN. If we get here,
      // the compiler generated wrong code.
      ASSERT(0, "Should not execute past end of bytecode.");
  }

  ASSERT(0, "Should not reach end of interpret.");
}

int wrenInterpret(WrenVM* vm, const char* sourcePath, const char* source)
{
  ObjFiber* fiber = wrenNewFiber(vm);
  PinnedObj pinned;
  pinObj(vm, (Obj*)fiber, &pinned);

  // TODO: Move actual error codes to main.c and return something Wren-specific
  // from here.
  int result = 0;
  ObjFn* fn = wrenCompile(vm, sourcePath, source);
  if (fn != NULL)
  {
    callFunction(fiber, (Obj*)fn, 0);
    if (!interpret(vm, fiber)) result = 70; // EX_SOFTWARE.
  }
  else
  {
    result = 65; // EX_DATAERR.
  }

  unpinObj(vm);
  return result;
}

void pinObj(WrenVM* vm, Obj* obj, PinnedObj* pinned)
{
  pinned->obj = obj;
  pinned->previous = vm->pinned;
  vm->pinned = pinned;
}

void unpinObj(WrenVM* vm)
{
  vm->pinned = vm->pinned->previous;
}

void wrenDefineMethod(WrenVM* vm, const char* className,
                      const char* methodName, int numParams,
                      WrenForeignMethodFn method)
{
  ASSERT(className != NULL, "Must provide class name.");

  int length = (int)strlen(methodName);
  ASSERT(methodName != NULL, "Must provide method name.");
  ASSERT(strlen(methodName) < MAX_METHOD_NAME, "Method name too long.");

  ASSERT(numParams >= 0, "numParams cannot be negative.");
  ASSERT(numParams <= MAX_PARAMETERS, "Too many parameters.");

  ASSERT(method != NULL, "Must provide method function.");

  // Find or create the class to bind the method to.
  int classSymbol = wrenSymbolTableFind(&vm->globalSymbols,
                               className, strlen(className));
  ObjClass* classObj;

  if (classSymbol != -1)
  {
    // TODO: Handle name is not class.
    classObj = AS_CLASS(vm->globals[classSymbol]);
  }
  else
  {
    // The class doesn't already exist, so create it.
    // TODO: Allow passing in name for superclass?
    classObj = wrenNewClass(vm, vm->objectClass, 0);
    classSymbol = wrenSymbolTableAdd(vm, &vm->globalSymbols,
                            className, strlen(className));
    vm->globals[classSymbol] = OBJ_VAL(classObj);
  }

  // Create a name for the method, including its arity.
  char name[MAX_METHOD_SIGNATURE];
  strncpy(name, methodName, length);
  for (int i = 0; i < numParams; i++)
  {
    name[length++] = ' ';
  }
  name[length] = '\0';

  // Bind the method.
  int methodSymbol = wrenSymbolTableEnsure(vm, &vm->methods, name, length);

  classObj->methods[methodSymbol].type = METHOD_FOREIGN;
  classObj->methods[methodSymbol].foreign = method;
}

double wrenGetArgumentDouble(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  // + 1 to shift past the receiver.
  // TODO: Check actual value type first.
  return AS_NUM(*(vm->foreignCallSlot + index + 1));
}

void wrenReturnDouble(WrenVM* vm, double value)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  *vm->foreignCallSlot = NUM_VAL(value);
  vm->foreignCallSlot = NULL;
}
