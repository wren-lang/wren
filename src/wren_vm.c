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

  WrenVM* vm = (WrenVM*)reallocate(NULL, 0, sizeof(WrenVM));

  vm->reallocate = reallocate;
  vm->foreignCallSlot = NULL;
  vm->foreignCallNumArgs = 0;
  vm->loadModule = configuration->loadModuleFn;

  wrenSymbolTableInit(vm, &vm->methodNames);

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
  vm->numTempRoots = 0;

  // Implicitly create a "main" module for the REPL or entry script.
  ObjModule* mainModule = wrenNewModule(vm);
  wrenPushRoot(vm, (Obj*)mainModule);

  vm->modules = wrenNewMap(vm);
  wrenMapSet(vm, vm->modules, NULL_VAL, OBJ_VAL(mainModule));

  wrenPopRoot(vm);

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

  if (vm->modules != NULL) wrenMarkObj(vm, (Obj*)vm->modules);

  // Temporary roots.
  for (int i = 0; i < vm->numTempRoots; i++)
  {
    wrenMarkObj(vm, vm->tempRoots[i]);
  }

  // The current fiber.
  if (vm->fiber != NULL) wrenMarkObj(vm, (Obj*)vm->fiber);

  // Any object the compiler is using (if there is one).
  if (vm->compiler != NULL) wrenMarkCompiler(vm, vm->compiler);

  // Collect any unmarked objects.
  Obj** obj = &vm->first;
  while (*obj != NULL)
  {
    if (!((*obj)->marked))
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
      (*obj)->marked = false;
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

// Captures the local variable [local] into an [Upvalue]. If that local is
// already in an upvalue, the existing one will be used. (This is important to
// ensure that multiple closures closing over the same variable actually see
// the same variable.) Otherwise, it will create a new open upvalue and add it
// the fiber's list of upvalues.
static Upvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, Value* local)
{
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
  method.fn.obj = AS_OBJ(methodValue);

  if (methodType == CODE_METHOD_STATIC)
  {
    classObj = classObj->obj.classObj;
  }

  wrenBindMethod(vm, classObj, symbol, method);
}

static void callForeign(WrenVM* vm, ObjFiber* fiber,
                        WrenForeignMethodFn foreign, int numArgs)
{
  vm->foreignCallSlot = fiber->stackTop - numArgs;
  vm->foreignCallNumArgs = numArgs;

  foreign(vm);

  // Discard the stack slots for the arguments (but leave one for
  // the result).
  fiber->stackTop -= numArgs - 1;

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
static ObjFiber* runtimeError(WrenVM* vm, ObjFiber* fiber, ObjString* error)
{
  ASSERT(fiber->error == NULL, "Can only fail once.");

  // Store the error in the fiber so it can be accessed later.
  fiber->error = error;

  // If the caller ran this fiber using "try", give it the error.
  if (fiber->callerIsTrying)
  {
    ObjFiber* caller = fiber->caller;

    // Make the caller's try method return the error message.
    *(caller->stackTop - 1) = OBJ_VAL(fiber->error);
    return caller;
  }

  // If we got here, nothing caught the error, so show the stack trace.
  wrenDebugPrintStackTrace(vm, fiber);
  return NULL;
}

// Creates a string containing an appropriate method not found error for a
// method with [symbol] on [classObj].
static ObjString* methodNotFound(WrenVM* vm, ObjClass* classObj, int symbol)
{
  // Count the number of spaces to determine the number of parameters the
  // method expects.
  const char* methodName = vm->methodNames.data[symbol].buffer;

  int methodLength = (int)strlen(methodName);
  int numParams = 0;
  while (methodName[methodLength - numParams - 1] == ' ') numParams++;

  char message[MAX_VARIABLE_NAME + MAX_METHOD_NAME + 49];
  sprintf(message, "%s does not implement method '%.*s' with %d argument%s.",
          classObj->name->value,
          methodLength - numParams, methodName,
          numParams,
          numParams == 1 ? "" : "s");

  return AS_STRING(wrenNewString(vm, message, strlen(message)));
}

// Pushes [function] onto [fiber]'s callstack and invokes it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
// [function] can be an `ObjFn` or `ObjClosure`.
static inline void callFunction(ObjFiber* fiber, Obj* function, int numArgs)
{
  // TODO: Check for stack overflow.
  CallFrame* frame = &fiber->frames[fiber->numFrames++];
  frame->fn = function;
  frame->stackStart = fiber->stackTop - numArgs;

  if (function->type == OBJ_FN)
  {
    frame->ip = ((ObjFn*)function)->bytecode;
  }
  else
  {
    frame->ip = ((ObjClosure*)function)->fn->bytecode;
  }
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
  register Value* stackStart;
  register uint8_t* ip;
  register ObjFn* fn;

  // These macros are designed to only be invoked within this function.
  #define PUSH(value)  (*fiber->stackTop++ = value)
  #define POP()        (*(--fiber->stackTop))
  #define DROP()       (fiber->stackTop--)
  #define PEEK()       (*(fiber->stackTop - 1))
  #define PEEK2()      (*(fiber->stackTop - 2))
  #define READ_BYTE()  (*ip++)
  #define READ_SHORT() (ip += 2, (ip[-2] << 8) | ip[-1])

  // Use this before a CallFrame is pushed to store the local variables back
  // into the current one.
  #define STORE_FRAME() frame->ip = ip

  // Use this after a CallFrame has been pushed or popped to refresh the local
  // variables.
  #define LOAD_FRAME()                                 \
      frame = &fiber->frames[fiber->numFrames - 1];    \
      stackStart = frame->stackStart;                  \
      ip = frame->ip;                                  \
      if (frame->fn->type == OBJ_FN)                   \
      {                                                \
        fn = (ObjFn*)frame->fn;                        \
      }                                                \
      else                                             \
      {                                                \
        fn = ((ObjClosure*)frame->fn)->fn;             \
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
    &&code_LOAD_LOCAL_0,
    &&code_LOAD_LOCAL_1,
    &&code_LOAD_LOCAL_2,
    &&code_LOAD_LOCAL_3,
    &&code_LOAD_LOCAL_4,
    &&code_LOAD_LOCAL_5,
    &&code_LOAD_LOCAL_6,
    &&code_LOAD_LOCAL_7,
    &&code_LOAD_LOCAL_8,
    &&code_LOAD_LOCAL,
    &&code_STORE_LOCAL,
    &&code_LOAD_UPVALUE,
    &&code_STORE_UPVALUE,
    &&code_LOAD_MODULE_VAR,
    &&code_STORE_MODULE_VAR,
    &&code_LOAD_FIELD_THIS,
    &&code_STORE_FIELD_THIS,
    &&code_LOAD_FIELD,
    &&code_STORE_FIELD,
    &&code_POP,
    &&code_DUP,
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

    #define DISPATCH()  goto *dispatchTable[instruction = (Code)READ_BYTE()];

  #endif

  #else

  #define INTERPRET_LOOP   loop: switch (instruction = (Code)READ_BYTE())
  #define CASE_CODE(name)  case CODE_##name
  #define DISPATCH()       goto loop

  #endif

  LOAD_FRAME();

  Code instruction;
  INTERPRET_LOOP
  {
    CASE_CODE(LOAD_LOCAL_0):
    CASE_CODE(LOAD_LOCAL_1):
    CASE_CODE(LOAD_LOCAL_2):
    CASE_CODE(LOAD_LOCAL_3):
    CASE_CODE(LOAD_LOCAL_4):
    CASE_CODE(LOAD_LOCAL_5):
    CASE_CODE(LOAD_LOCAL_6):
    CASE_CODE(LOAD_LOCAL_7):
    CASE_CODE(LOAD_LOCAL_8):
      PUSH(stackStart[instruction - CODE_LOAD_LOCAL_0]);
      DISPATCH();

    CASE_CODE(LOAD_LOCAL):
      PUSH(stackStart[READ_BYTE()]);
      DISPATCH();

    CASE_CODE(LOAD_FIELD_THIS):
    {
      uint8_t field = READ_BYTE();
      Value receiver = stackStart[0];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(POP):   DROP(); DISPATCH();
    CASE_CODE(DUP):
    {
      Value value = PEEK();
      PUSH(value); DISPATCH();
    }

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

      // The receiver is the first argument.
      Value* args = fiber->stackTop - numArgs;
      ObjClass* classObj = wrenGetClassInline(vm, args[0]);

      // If the class's method table doesn't include the symbol, bail.
      if (symbol >= classObj->methods.count)
      {
        RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));
      }

      Method* method = &classObj->methods.data[symbol];
      switch (method->type)
      {
        case METHOD_PRIMITIVE:
        {
          // After calling this, the result will be in the first arg slot.
          switch (method->fn.primitive(vm, fiber, args))
          {
            case PRIM_VALUE:
              // The result is now in the first arg slot. Discard the other
              // stack slots.
              fiber->stackTop -= numArgs - 1;
              break;

            case PRIM_ERROR:
              RUNTIME_ERROR(AS_STRING(args[0]));

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
          callForeign(vm, fiber, method->fn.foreign, numArgs);
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(fiber, method->fn.obj, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));
          break;
      }
      DISPATCH();
    }

    CASE_CODE(STORE_LOCAL):
      stackStart[READ_BYTE()] = PEEK();
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

      // The receiver is the first argument.
      Value* args = fiber->stackTop - numArgs;
      ObjClass* classObj = wrenGetClassInline(vm, args[0]);

      // Ignore methods defined on the receiver's immediate class.
      classObj = classObj->superclass;

      // If the class's method table doesn't include the symbol, bail.
      if (symbol >= classObj->methods.count)
      {
        RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));
      }

      Method* method = &classObj->methods.data[symbol];
      switch (method->type)
      {
        case METHOD_PRIMITIVE:
        {
          // After calling this, the result will be in the first arg slot.
          switch (method->fn.primitive(vm, fiber, args))
          {
            case PRIM_VALUE:
              // The result is now in the first arg slot. Discard the other
              // stack slots.
              fiber->stackTop -= numArgs - 1;
              break;

            case PRIM_ERROR:
              RUNTIME_ERROR(AS_STRING(args[0]));

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
          callForeign(vm, fiber, method->fn.foreign, numArgs);
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(fiber, method->fn.obj, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));
          break;
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_UPVALUE):
    {
      Upvalue** upvalues = ((ObjClosure*)frame->fn)->upvalues;
      PUSH(*upvalues[READ_BYTE()]->value);
      DISPATCH();
    }

    CASE_CODE(STORE_UPVALUE):
    {
      Upvalue** upvalues = ((ObjClosure*)frame->fn)->upvalues;
      *upvalues[READ_BYTE()]->value = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_MODULE_VAR):
      PUSH(fn->module->variables.data[READ_SHORT()]);
      DISPATCH();

    CASE_CODE(STORE_MODULE_VAR):
      fn->module->variables.data[READ_SHORT()] = PEEK();
      DISPATCH();

    CASE_CODE(STORE_FIELD_THIS):
    {
      uint8_t field = READ_BYTE();
      Value receiver = stackStart[0];
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(LOAD_FIELD):
    {
      uint8_t field = READ_BYTE();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
      PUSH(instance->fields[field]);
      DISPATCH();
    }

    CASE_CODE(STORE_FIELD):
    {
      uint8_t field = READ_BYTE();
      Value receiver = POP();
      ASSERT(IS_INSTANCE(receiver), "Receiver should be instance.");
      ObjInstance* instance = AS_INSTANCE(receiver);
      ASSERT(field < instance->obj.classObj->numFields, "Out of bounds field.");
      instance->fields[field] = PEEK();
      DISPATCH();
    }

    CASE_CODE(JUMP):
    {
      uint16_t offset = READ_SHORT();
      ip += offset;
      DISPATCH();
    }

    CASE_CODE(LOOP):
    {
      // Jump back to the top of the loop.
      uint16_t offset = READ_SHORT();
      ip -= offset;
      DISPATCH();
    }

    CASE_CODE(JUMP_IF):
    {
      uint16_t offset = READ_SHORT();
      Value condition = POP();

      if (IS_FALSE(condition) || IS_NULL(condition)) ip += offset;
      DISPATCH();
    }

    CASE_CODE(AND):
    {
      uint16_t offset = READ_SHORT();
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
      uint16_t offset = READ_SHORT();
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
      if (!IS_CLASS(expected))
      {
        const char* message = "Right operand must be a class.";
        RUNTIME_ERROR(AS_STRING(wrenNewString(vm, message, strlen(message))));
      }

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
      Value* firstValue = stackStart;
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
        *(fiber->stackTop - 1) = result;
      }
      else
      {
        // Store the result of the block in the first slot, which is where the
        // caller expects it.
        stackStart[0] = result;

        // Discard the stack slots for the call frame (leaving one slot for the
        // result).
        fiber->stackTop = frame->stackStart + 1;
      }

      LOAD_FRAME();
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
        uint8_t isLocal = READ_BYTE();
        uint8_t index = READ_BYTE();
        if (isLocal)
        {
          // Make an new upvalue to close over the parent's local variable.
          closure->upvalues[i] = captureUpvalue(vm, fiber,
                                                frame->stackStart + index);
        }
        else
        {
          // Use the same upvalue as the current call frame.
          closure->upvalues[i] = ((ObjClosure*)frame->fn)->upvalues[index];
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
        sprintf(message,
            "Class '%s' may not have more than %d fields, including inherited "
            "ones.", name->value, MAX_FIELDS);

        RUNTIME_ERROR(AS_STRING(wrenNewString(vm, message, strlen(message))));
      }

      PUSH(OBJ_VAL(classObj));
      DISPATCH();
    }

    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      uint16_t symbol = READ_SHORT();
      ObjClass* classObj = AS_CLASS(PEEK());
      Value method = PEEK2();
      bindMethod(vm, instruction, symbol, classObj, method);
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

// Looks up the core module in the module map.
static ObjModule* getCoreModule(WrenVM* vm)
{
  uint32_t entry = wrenMapFind(vm->modules, NULL_VAL);
  ASSERT(entry != UINT32_MAX, "Could not find core module.");
  return AS_MODULE(vm->modules->entries[entry].value);
}

static ObjFiber* loadModule(WrenVM* vm, Value name, const char* source)
{
  ObjModule* module = wrenNewModule(vm);
  wrenPushRoot(vm, (Obj*)module);

  // Implicitly import the core module.
  ObjModule* coreModule = getCoreModule(vm);
  for (int i = 0; i < coreModule->variables.count; i++)
  {
    wrenDefineVariable(vm, module,
                       coreModule->variableNames.data[i].buffer,
                       coreModule->variableNames.data[i].length,
                       coreModule->variables.data[i]);
  }

  ObjFn* fn = wrenCompile(vm, module, AS_CSTRING(name), source);
  if (fn == NULL)
  {
    // TODO: Should we still store the module even if it didn't compile?
    wrenPopRoot(vm); // module.
    return NULL;
  }

  wrenPushRoot(vm, (Obj*)fn);

  // Store it in the VM's module registry so we don't load the same module
  // multiple times.
  wrenMapSet(vm, vm->modules, name, OBJ_VAL(module));

  ObjFiber* moduleFiber = wrenNewFiber(vm, (Obj*)fn);

  wrenPopRoot(vm); // fn.
  wrenPopRoot(vm); // module.

  // Return the fiber that executes the module.
  return moduleFiber;
}

// Execute [source] in the context of the core module.
WrenInterpretResult static loadIntoCore(WrenVM* vm, const char* source)
{
  ObjModule* coreModule = getCoreModule(vm);

  ObjFn* fn = wrenCompile(vm, coreModule, "", source);
  if (fn == NULL) return WREN_RESULT_COMPILE_ERROR;

  wrenPushRoot(vm, (Obj*)fn);
  vm->fiber = wrenNewFiber(vm, (Obj*)fn);
  wrenPopRoot(vm); // fn.

  return runInterpreter(vm) ? WREN_RESULT_SUCCESS : WREN_RESULT_RUNTIME_ERROR;
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* sourcePath,
                                  const char* source)
{
  if (strlen(sourcePath) == 0) return loadIntoCore(vm, source);
  
  // TODO: Better module name.
  Value name = wrenNewString(vm, "main", 4);
  wrenPushRoot(vm, AS_OBJ(name));

  ObjFiber* fiber = loadModule(vm, name, source);
  if (fiber == NULL)
  {
    wrenPopRoot(vm);
    return WREN_RESULT_COMPILE_ERROR;
  }

  vm->fiber = fiber;

  bool succeeded = runInterpreter(vm);

  wrenPopRoot(vm); // name.

  return succeeded ? WREN_RESULT_SUCCESS : WREN_RESULT_RUNTIME_ERROR;
}

Value wrenImportModule(WrenVM* vm, const char* name)
{
  Value nameValue = wrenNewString(vm, name, strlen(name));
  wrenPushRoot(vm, AS_OBJ(nameValue));

  // If the module is already loaded, we don't need to do anything.
  uint32_t index = wrenMapFind(vm->modules, nameValue);
  if (index != UINT32_MAX)
  {
    wrenPopRoot(vm); // nameValue.
    return NULL_VAL;
  }

  // Load the module's source code from the embedder.
  char* source = vm->loadModule(vm, name);
  if (source == NULL)
  {
    wrenPopRoot(vm); // nameValue.

    // Couldn't load the module.
    Value error = wrenNewUninitializedString(vm, 25 + strlen(name));
    sprintf(AS_STRING(error)->value, "Could not find module '%s'.", name);
    return error;
  }

  ObjFiber* moduleFiber = loadModule(vm, nameValue, source);

  wrenPopRoot(vm); // nameValue.

  // Return the fiber that executes the module.
  return OBJ_VAL(moduleFiber);
}

Value wrenFindVariable(WrenVM* vm, const char* name)
{
  ObjModule* coreModule = getCoreModule(vm);
  int symbol = wrenSymbolTableFind(&coreModule->variableNames,
                                   name, strlen(name));
  return coreModule->variables.data[symbol];
}

int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length)
{
  if (module == NULL) module = getCoreModule(vm);
  if (module->variables.count == MAX_MODULE_VARS) return -2;

  wrenValueBufferWrite(vm, &module->variables, UNDEFINED_VAL);
  return wrenSymbolTableAdd(vm, &module->variableNames, name, length);
}

int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,
                       size_t length, Value value)
{
  if (module == NULL) module = getCoreModule(vm);
  if (module->variables.count == MAX_MODULE_VARS) return -2;

  if (IS_OBJ(value)) wrenPushRoot(vm, AS_OBJ(value));

  // See if the variable is already explicitly or implicitly declared.
  int symbol = wrenSymbolTableFind(&module->variableNames, name, length);

  if (symbol == -1)
  {
    // Brand new variable.
    symbol = wrenSymbolTableAdd(vm, &module->variableNames, name, length);
    wrenValueBufferWrite(vm, &module->variables, value);
  }
  else if (IS_UNDEFINED(module->variables.data[symbol]))
  {
    // Explicitly declaring an implicitly declared one. Mark it as defined.
    module->variables.data[symbol] = value;
  }
  else
  {
    // Already explicitly declared.
    symbol = -1;
  }

  if (IS_OBJ(value)) wrenPopRoot(vm);

  return symbol;
}

// TODO: Inline?
void wrenPushRoot(WrenVM* vm, Obj* obj)
{
  ASSERT(obj != NULL, "Can't root NULL.");
  ASSERT(vm->numTempRoots < WREN_MAX_TEMP_ROOTS, "Too many temporary roots.");

  vm->tempRoots[vm->numTempRoots++] = obj;
}

void wrenPopRoot(WrenVM* vm)
{
  ASSERT(vm->numTempRoots > 0, "No temporary roots to release.");
  vm->numTempRoots--;
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

  // TODO: Need to be able to define methods in classes outside the core
  // module.
  
  // Find or create the class to bind the method to.
  ObjModule* coreModule = getCoreModule(vm);
  int classSymbol = wrenSymbolTableFind(&coreModule->variableNames,
                                        className, strlen(className));
  ObjClass* classObj;

  if (classSymbol != -1)
  {
    // TODO: Handle name is not class.
    classObj = AS_CLASS(coreModule->variables.data[classSymbol]);
  }
  else
  {
    // The class doesn't already exist, so create it.
    size_t length = strlen(className);
    ObjString* nameString = AS_STRING(wrenNewString(vm, className, length));

    wrenPushRoot(vm, (Obj*)nameString);

    // TODO: Allow passing in name for superclass?
    classObj = wrenNewClass(vm, vm->objectClass, 0, nameString);
    wrenDefineVariable(vm, coreModule, className, length, OBJ_VAL(classObj));

    wrenPopRoot(vm);
  }

  // Create a name for the method, including its arity.
  char name[MAX_METHOD_SIGNATURE];
  memcpy(name, methodName, length);
  for (int i = 0; i < numParams; i++)
  {
    name[length++] = ' ';
  }
  name[length] = '\0';

  // Bind the method.
  int methodSymbol = wrenSymbolTableEnsure(vm, &vm->methodNames, name, length);

  Method method;
  method.type = METHOD_FOREIGN;
  method.fn.foreign = methodFn;

  if (isStatic) classObj = classObj->obj.classObj;

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

bool wrenGetArgumentBool(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  if (!IS_BOOL(*(vm->foreignCallSlot + index))) return false;

  return AS_BOOL(*(vm->foreignCallSlot + index));
}

double wrenGetArgumentDouble(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  if (!IS_NUM(*(vm->foreignCallSlot + index))) return 0.0;

  return AS_NUM(*(vm->foreignCallSlot + index));
}

const char* wrenGetArgumentString(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");

  if (!IS_STRING(*(vm->foreignCallSlot + index))) return NULL;

  return AS_CSTRING(*(vm->foreignCallSlot + index));
}

void wrenReturnBool(WrenVM* vm, bool value)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  *vm->foreignCallSlot = BOOL_VAL(value);
  vm->foreignCallSlot = NULL;
}

void wrenReturnDouble(WrenVM* vm, double value)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  *vm->foreignCallSlot = NUM_VAL(value);
  vm->foreignCallSlot = NULL;
}

void wrenReturnString(WrenVM* vm, const char* text, int length)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(text != NULL, "String cannot be NULL.");
  
  size_t size = length;
  if (length == -1) size = strlen(text);

  *vm->foreignCallSlot = wrenNewString(vm, text, size);
  vm->foreignCallSlot = NULL;
}
