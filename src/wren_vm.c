#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_core.h"
#include "wren_debug.h"
#include "wren_vm.h"

#if WREN_USE_LIB_IO
  #include "wren_io.h"
#endif

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
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

  wrenSymbolTableInit(vm, &vm->methodNames);
  wrenSymbolTableInit(vm, &vm->globalNames);
  wrenValueBufferInit(vm, &vm->globals);

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
  vm->fiber = NULL;
  vm->first = NULL;
  vm->pinned = NULL;

  vm->foreignCallSlot = NULL;
  vm->foreignCallNumArgs = 0;

  wrenInitializeCore(vm);
  #if WREN_USE_LIB_IO
    wrenLoadIOLibrary(vm);
  #endif

  return vm;
}

void wrenFreeVM(WrenVM* vm)
{
  // TODO: Check for already freed.
  // Free all of the GC objects.
  Obj* obj = vm->first;
  while (obj != NULL)
  {
    Obj* next = obj->next;
    wrenFreeObj(vm, obj);
    obj = next;
  }

  wrenSymbolTableClear(vm, &vm->methodNames);
  wrenSymbolTableClear(vm, &vm->globalNames);
  wrenValueBufferClear(vm, &vm->globals);

  wrenReallocate(vm, vm, 0, 0);
}

static void collectGarbage(WrenVM* vm);

void wrenSetCompiler(WrenVM* vm, Compiler* compiler)
{
  vm->compiler = compiler;
}

static void collectGarbage(WrenVM* vm)
{
#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
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
  for (int i = 0; i < vm->globals.count; i++)
  {
    wrenMarkValue(vm, vm->globals.data[i]);
  }

  // Pinned objects.
  WrenPinnedObj* pinned = vm->pinned;
  while (pinned != NULL)
  {
    wrenMarkObj(vm, pinned->obj);
    pinned = pinned->previous;
  }

  // The current fiber.
  if (vm->fiber != NULL) wrenMarkObj(vm, (Obj*)vm->fiber);

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

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
  double elapsed = ((double)clock() / CLOCKS_PER_SEC) - startTime;
  printf("GC %ld before, %ld after (%ld collected), next at %ld. Took %.3fs.\n",
         before, vm->bytesAllocated, before - vm->bytesAllocated, vm->nextGC,
         elapsed);
#endif
}

void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize)
{
#if WREN_DEBUG_TRACE_MEMORY
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

  // Walk towards the bottom of the stack until we find a previously existing
  // upvalue or pass where it should be.
  while (upvalue != NULL && upvalue->value > local)
  {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  // Found an existing upvalue for this local.
  if (upvalue != NULL && upvalue->value == local) return upvalue;

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

static void bindMethod(WrenVM* vm, int methodType, int symbol,
                       ObjClass* classObj, Value methodValue)
{
  ObjFn* methodFn = IS_FN(methodValue) ? AS_FN(methodValue)
                                       : AS_CLOSURE(methodValue)->fn;

  // Methods are always bound against the class, and not the metaclass, even
  // for static methods, so that constructors (which are static) get bound like
  // instance methods.
  wrenBindMethodCode(classObj, methodFn);

  Method method;
  method.type = METHOD_BLOCK;
  method.fn = AS_OBJ(methodValue);

  if (methodType == CODE_METHOD_STATIC)
  {
    classObj = classObj->metaclass;
  }

  wrenBindMethod(vm, classObj, symbol, method);
}

static void callForeign(WrenVM* vm, ObjFiber* fiber,
                        WrenForeignMethodFn foreign, int numArgs)
{
  vm->foreignCallSlot = &fiber->stack[fiber->stackSize - numArgs];
  vm->foreignCallNumArgs = numArgs;

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

// Puts [fiber] into a runtime failed state because of [error].
//
// Returns the fiber that should receive the error or `NULL` if no fiber
// caught it.
static ObjFiber* runtimeError(WrenVM* vm, ObjFiber* fiber, const char* error)
{
  ASSERT(fiber->error == NULL, "Can only fail once.");

  // Store the error in the fiber so it can be accessed later.
  fiber->error = AS_STRING(wrenNewString(vm, error, strlen(error)));

  // If the caller ran this fiber using "try", give it the error.
  if (fiber->callerIsTrying)
  {
    ObjFiber* caller = fiber->caller;

    // Make the caller's try method return the error message.
    caller->stack[caller->stackSize - 1] = OBJ_VAL(fiber->error);

    return caller;
  }

  // If we got here, nothing caught the error, so show the stack trace.
  wrenDebugPrintStackTrace(vm, fiber);
  return NULL;
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
static bool runInterpreter(WrenVM* vm)
{
  // Hoist these into local variables. They are accessed frequently in the loop
  // but assigned less frequently. Keeping them in locals and updating them when
  // a call frame has been pushed or popped gives a large speed boost.
  register ObjFiber* fiber = vm->fiber;
  register CallFrame* frame;
  register uint8_t* ip;
  register ObjFn* fn;
  register Upvalue** upvalues;

  // These macros are designed to only be invoked within this function.
  #define PUSH(value)  (fiber->stack[fiber->stackSize++] = value)
  #define POP()        (fiber->stack[--fiber->stackSize])
  #define DROP()       (fiber->stackSize--)
  #define PEEK()       (fiber->stack[fiber->stackSize - 1])
  #define PEEK2()      (fiber->stack[fiber->stackSize - 2])
  #define READ_BYTE()  (*ip++)
  #define READ_SHORT() (ip += 2, (ip[-2] << 8) | ip[-1])

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

  // Terminates the current fiber with error string [error]. If another calling
  // fiber is willing to catch the error, transfers control to it, otherwise
  // exits the interpreter.
  #define RUNTIME_ERROR(error)                         \
      do {                                             \
        STORE_FRAME();                                 \
        fiber = runtimeError(vm, fiber, error);        \
        if (fiber == NULL) return false;               \
        LOAD_FRAME();                                  \
        DISPATCH();                                    \
      }                                                \
      while (false)

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
    &&code_LIST,
    &&code_CLOSURE,
    &&code_CLASS,
    &&code_METHOD_INSTANCE,
    &&code_METHOD_STATIC,
    &&code_END
  };

  #define INTERPRET_LOOP    DISPATCH();
  #define CASE_CODE(name)   code_##name

    #if WREN_DEBUG_TRACE_INSTRUCTIONS
      // Prints the stack and instruction before each instruction is executed.
      #define DISPATCH() \
          { \
            wrenDebugPrintStack(fiber); \
            wrenDebugPrintInstruction(vm, fn, (int)(ip - fn->bytecode)); \
            instruction = *ip++; \
            goto *dispatchTable[instruction]; \
          }
    #else
      #define DISPATCH()        goto *dispatchTable[instruction = READ_BYTE()]
    #endif

  #else

  #define INTERPRET_LOOP    for (;;) switch (instruction = READ_BYTE())
  #define CASE_CODE(name)   case CODE_##name
  #define DISPATCH()        break

  #endif

  LOAD_FRAME();

  Code instruction;
  INTERPRET_LOOP
  {
    CASE_CODE(POP):   DROP(); DISPATCH();
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
      int symbol = READ_SHORT();

      Value receiver = fiber->stack[fiber->stackSize - numArgs];
      ObjClass* classObj = wrenGetClass(vm, receiver);

      // If the class's method table doesn't include the symbol, bail.
      if (symbol >= classObj->methods.count)
      {
        char message[100];
        snprintf(message, 100, "%s does not implement method '%s'.",
                 classObj->name->value,
                 vm->methodNames.data[symbol]);
        RUNTIME_ERROR(message);
      }

      Method* method = &classObj->methods.data[symbol];
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
              RUNTIME_ERROR(AS_CSTRING(args[0]));

            case PRIM_CALL:
              STORE_FRAME();
              callFunction(fiber, AS_OBJ(args[0]), numArgs);
              LOAD_FRAME();
              break;

            case PRIM_RUN_FIBER:
              STORE_FRAME();
              fiber = AS_FIBER(args[0]);
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
        {
          char message[100];
          snprintf(message, 100, "%s does not implement method '%s'.",
                   classObj->name->value,
                   vm->methodNames.data[symbol]);
          RUNTIME_ERROR(message);
        }
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_LOCAL):
      PUSH(fiber->stack[frame->stackStart + READ_BYTE()]);
      DISPATCH();

    CASE_CODE(STORE_LOCAL):
      fiber->stack[frame->stackStart + READ_BYTE()] = PEEK();
      DISPATCH();

    CASE_CODE(CONSTANT):
      PUSH(fn->constants[READ_SHORT()]);
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
      int symbol = READ_SHORT();

      Value receiver = fiber->stack[fiber->stackSize - numArgs];
      ObjClass* classObj = wrenGetClass(vm, receiver);

      // Ignore methods defined on the receiver's immediate class.
      classObj = classObj->superclass;

      // If the class's method table doesn't include the symbol, bail.
      if (symbol >= classObj->methods.count)
      {
        char message[100];
        snprintf(message, 100, "%s does not implement method '%s'.",
                 classObj->name->value, vm->methodNames.data[symbol]);
        RUNTIME_ERROR(message);
      }

      Method* method = &classObj->methods.data[symbol];
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
              RUNTIME_ERROR(AS_CSTRING(args[0]));

            case PRIM_CALL:
              STORE_FRAME();
              callFunction(fiber, AS_OBJ(args[0]), numArgs);
              LOAD_FRAME();
              break;

            case PRIM_RUN_FIBER:
              STORE_FRAME();
              fiber = AS_FIBER(args[0]);
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
        {
          char message[100];
          snprintf(message, 100, "%s does not implement method '%s'.",
                   classObj->name->value, vm->methodNames.data[symbol]);
          RUNTIME_ERROR(message);
        }
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_UPVALUE):
      ASSERT(upvalues != NULL,
             "Should not have CODE_LOAD_UPVALUE instruction in non-closure.");
      PUSH(*upvalues[READ_BYTE()]->value);
      DISPATCH();

    CASE_CODE(STORE_UPVALUE):
      ASSERT(upvalues != NULL,
             "Should not have CODE_STORE_UPVALUE instruction in non-closure.");
      *upvalues[READ_BYTE()]->value = PEEK();
      DISPATCH();

    CASE_CODE(LOAD_GLOBAL):
      PUSH(vm->globals.data[READ_SHORT()]);
      DISPATCH();

    CASE_CODE(STORE_GLOBAL):
      vm->globals.data[READ_SHORT()] = PEEK();
      DISPATCH();

    CASE_CODE(LOAD_FIELD_THIS):
    {
      int field = READ_BYTE();
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD_THIS):
    {
      int field = READ_BYTE();
      Value receiver = fiber->stack[frame->stackStart];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_FIELD):
    {
      int field = READ_BYTE();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD):
    {
      int field = READ_BYTE();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(JUMP):
    {
      int offset = READ_SHORT();
      ip += offset;
      DISPATCH();
    }

    CASE_CODE(LOOP):
    {
      // Jump back to the top of the loop.
      int offset = READ_SHORT();
      ip -= offset;
      DISPATCH();
    }

    CASE_CODE(JUMP_IF):
    {
      int offset = READ_SHORT();
      Value condition = POP();

      if (IS_FALSE(condition) || IS_NULL(condition)) ip += offset;
      DISPATCH();
    }

    CASE_CODE(AND):
    {
      int offset = READ_SHORT();
      Value condition = PEEK();

      if (IS_FALSE(condition) || IS_NULL(condition))
      {
        // Short-circuit the right hand side.
        ip += offset;
      }
      else
      {
        // Discard the condition and evaluate the right hand side.
        DROP();
      }
      DISPATCH();
    }

    CASE_CODE(OR):
    {
      int offset = READ_SHORT();
      Value condition = PEEK();

      if (IS_FALSE(condition) || IS_NULL(condition))
      {
        // Discard the condition and evaluate the right hand side.
        DROP();
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
      Value expected = POP();
      if (!IS_CLASS(expected)) RUNTIME_ERROR("Right operand must be a class.");

      ObjClass* actual = wrenGetClass(vm, POP());
      bool isInstance = false;

      // Walk the superclass chain looking for the class.
      while (actual != NULL)
      {
        if (actual == AS_CLASS(expected))
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
      DROP();
      DISPATCH();

    CASE_CODE(RETURN):
    {
      Value result = POP();
      fiber->numFrames--;

      // Close any upvalues still in scope.
      Value* firstValue = &fiber->stack[frame->stackStart];
      while (fiber->openUpvalues != NULL &&
             fiber->openUpvalues->value >= firstValue)
      {
        closeUpvalue(fiber);
      }

      // If the fiber is complete, end it.
      if (fiber->numFrames == 0)
      {
        // If this is the main fiber, we're done.
        if (fiber->caller == NULL) return true;

        // We have a calling fiber to resume.
        fiber = fiber->caller;

        // Store the result in the resuming fiber.
        fiber->stack[fiber->stackSize - 1] = result;
      }
      else
      {
        // Store the result of the block in the first slot, which is where the
        // caller expects it.
        fiber->stack[frame->stackStart] = result;

        // Discard the stack slots for the call frame (leaving one slot for the
        // result).
        fiber->stackSize = frame->stackStart + 1;
      }

      LOAD_FRAME();
      DISPATCH();
    }

    CASE_CODE(LIST):
    {
      int numElements = READ_BYTE();
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
      ObjFn* prototype = AS_FN(fn->constants[READ_SHORT()]);

      ASSERT(prototype->numUpvalues > 0,
             "Should not create closure for functions that don't need it.");

      // Create the closure and push it on the stack before creating upvalues
      // so that it doesn't get collected.
      ObjClosure* closure = wrenNewClosure(vm, prototype);
      PUSH(OBJ_VAL(closure));

      // Capture upvalues.
      for (int i = 0; i < prototype->numUpvalues; i++)
      {
        bool isLocal = READ_BYTE();
        int index = READ_BYTE();
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
    {
      ObjString* name = AS_STRING(PEEK2());

      ObjClass* superclass;
      if (IS_NULL(PEEK()))
      {
        // Implicit Object superclass.
        superclass = vm->objectClass;
      }
      else
      {
        // TODO: Handle the superclass not being a class object!
        superclass = AS_CLASS(PEEK());
      }

      int numFields = READ_BYTE();

      ObjClass* classObj = wrenNewClass(vm, superclass, numFields, name);

      // Don't pop the superclass and name off the stack until the subclass is
      // done being created, to make sure it doesn't get collected.
      DROP();
      DROP();

      // Now that we know the total number of fields, make sure we don't
      // overflow.
      if (superclass->numFields + numFields > MAX_FIELDS)
      {
        char message[70 + MAX_VARIABLE_NAME];
        snprintf(message, 70 + MAX_VARIABLE_NAME,
            "Class '%s' may not have more than %d fields, including inherited "
            "ones.", name->value, MAX_FIELDS);
        RUNTIME_ERROR(message);
      }

      PUSH(OBJ_VAL(classObj));
      DISPATCH();
    }

    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      int type = instruction;
      int symbol = READ_SHORT();
      ObjClass* classObj = AS_CLASS(PEEK());
      Value method = PEEK2();
      bindMethod(vm, type, symbol, classObj, method);
      DROP();
      DROP();
      DISPATCH();
    }

    CASE_CODE(END):
      // A CODE_END should always be preceded by a CODE_RETURN. If we get here,
      // the compiler generated wrong code.
      UNREACHABLE();
  }

  // We should only exit this function from an explicit return from CODE_RETURN
  // or a runtime error.
  UNREACHABLE();
  return false;
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* sourcePath,
                                  const char* source)
{
  // TODO: Check for freed VM.
  ObjFn* fn = wrenCompile(vm, sourcePath, source);
  if (fn == NULL) return WREN_RESULT_COMPILE_ERROR;

  WREN_PIN(vm, fn);
  vm->fiber = wrenNewFiber(vm, (Obj*)fn);
  WREN_UNPIN(vm);

  if (runInterpreter(vm))
  {
    return WREN_RESULT_SUCCESS;
  }
  else
  {
    return WREN_RESULT_RUNTIME_ERROR;
  }
}

int wrenDefineGlobal(WrenVM* vm, const char* name, size_t length, Value value)
{
  if (vm->globals.count == MAX_GLOBALS) return -2;

  if (IS_OBJ(value)) WREN_PIN(vm, AS_OBJ(value));

  int symbol = wrenSymbolTableAdd(vm, &vm->globalNames, name, length);
  if (symbol != -1) wrenValueBufferWrite(vm, &vm->globals, value);

  if (IS_OBJ(value)) WREN_UNPIN(vm);

  return symbol;
}

void wrenPinObj(WrenVM* vm, Obj* obj, WrenPinnedObj* pinned)
{
  pinned->obj = obj;
  pinned->previous = vm->pinned;
  vm->pinned = pinned;
}

void wrenUnpinObj(WrenVM* vm)
{
  vm->pinned = vm->pinned->previous;
}

static void defineMethod(WrenVM* vm, const char* className,
                         const char* methodName, int numParams,
                         WrenForeignMethodFn methodFn, bool isStatic)
{
  ASSERT(className != NULL, "Must provide class name.");

  int length = (int)strlen(methodName);
  ASSERT(methodName != NULL, "Must provide method name.");
  ASSERT(strlen(methodName) < MAX_METHOD_NAME, "Method name too long.");

  ASSERT(numParams >= 0, "numParams cannot be negative.");
  ASSERT(numParams <= MAX_PARAMETERS, "Too many parameters.");

  ASSERT(methodFn != NULL, "Must provide method function.");

  // Find or create the class to bind the method to.
  int classSymbol = wrenSymbolTableFind(&vm->globalNames,
                               className, strlen(className));
  ObjClass* classObj;

  if (classSymbol != -1)
  {
    // TODO: Handle name is not class.
    classObj = AS_CLASS(vm->globals.data[classSymbol]);
  }
  else
  {
    // The class doesn't already exist, so create it.
    size_t length = strlen(className);
    ObjString* nameString = AS_STRING(wrenNewString(vm, className, length));

    WREN_PIN(vm, nameString);

    // TODO: Allow passing in name for superclass?
    classObj = wrenNewClass(vm, vm->objectClass, 0, nameString);
    wrenDefineGlobal(vm, className, length, OBJ_VAL(classObj));

    WREN_UNPIN(vm);
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
  int methodSymbol = wrenSymbolTableEnsure(vm, &vm->methodNames, name, length);

  Method method;
  method.type = METHOD_FOREIGN;
  method.foreign = methodFn;

  if (isStatic) classObj = classObj->metaclass;

  wrenBindMethod(vm, classObj, methodSymbol, method);
}

void wrenDefineMethod(WrenVM* vm, const char* className,
                      const char* methodName, int numParams,
                      WrenForeignMethodFn methodFn)
{
  defineMethod(vm, className, methodName, numParams, methodFn, false);
}

void wrenDefineStaticMethod(WrenVM* vm, const char* className,
                            const char* methodName, int numParams,
                            WrenForeignMethodFn methodFn)
{
  defineMethod(vm, className, methodName, numParams, methodFn, true);
}

double wrenGetArgumentDouble(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  // TODO: Check actual value type first.
  return AS_NUM(*(vm->foreignCallSlot + index));
}

const char* wrenGetArgumentString(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  // TODO: Check actual value type first.
  return AS_CSTRING(*(vm->foreignCallSlot + index));
}

void wrenReturnDouble(WrenVM* vm, double value)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  *vm->foreignCallSlot = NUM_VAL(value);
  vm->foreignCallSlot = NULL;
}

void wrenReturnNull(WrenVM* vm)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  *vm->foreignCallSlot = NULL_VAL;
  vm->foreignCallSlot = NULL;
}

void wrenReturnString(WrenVM* vm, const char* text, int length)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  size_t size = length;
  if (length == -1) size = strlen(text);

  *vm->foreignCallSlot = wrenNewString(vm, text, size);
  vm->foreignCallSlot = NULL;
}
