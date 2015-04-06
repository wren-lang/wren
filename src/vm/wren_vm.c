#include <stdarg.h>
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

#if WREN_USE_LIB_META
  #include "wren_meta.h"
#endif

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
  #include <time.h>
#endif

WrenVM* wrenNewVM(WrenConfiguration* configuration)
{
  WrenReallocateFn reallocate = realloc;
  if (configuration->reallocateFn != NULL)
  {
    reallocate = configuration->reallocateFn;
  }

  WrenVM* vm = (WrenVM*)reallocate(NULL, sizeof(*vm));
  memset(vm, 0, sizeof(WrenVM));

  vm->reallocate = reallocate;
  vm->bindForeign = configuration->bindForeignMethodFn;
  vm->loadModule = configuration->loadModuleFn;

  wrenSymbolTableInit(&vm->methodNames);

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

  ObjString* name = AS_STRING(CONST_STRING(vm, "core"));
  wrenPushRoot(vm, (Obj*)name);

  // Implicitly create a "core" module for the built in libraries.
  ObjModule* coreModule = wrenNewModule(vm, name);
  wrenPushRoot(vm, (Obj*)coreModule);

  vm->modules = wrenNewMap(vm);
  wrenMapSet(vm, vm->modules, NULL_VAL, OBJ_VAL(coreModule));

  wrenPopRoot(vm); // mainModule.
  wrenPopRoot(vm); // name.

  wrenInitializeCore(vm);
  #if WREN_USE_LIB_IO
    wrenLoadIOLibrary(vm);
  #endif
  #if WREN_USE_LIB_META
    wrenLoadMetaLibrary(vm);
  #endif

  return vm;
}

void wrenFreeVM(WrenVM* vm)
{
  ASSERT(vm->methodNames.count > 0, "VM appears to have already been freed.");

  // Free all of the GC objects.
  Obj* obj = vm->first;
  while (obj != NULL)
  {
    Obj* next = obj->next;
    wrenFreeObj(vm, obj);
    obj = next;
  }

  // Tell the user if they didn't free any method handles. We don't want to
  // just free them here because the host app may still have pointers to them
  // that they may try to use. Better to tell them about the bug early.
  ASSERT(vm->methodHandles == NULL, "All methods have not been released.");

  wrenSymbolTableClear(vm, &vm->methodNames);

  DEALLOCATE(vm, vm);
}

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
  // we need to know its class to know how big it is, but its class may have
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

  // The method handles.
  for (WrenMethod* handle = vm->methodHandles;
       handle != NULL;
       handle = handle->next)
  {
    wrenMarkObj(vm, (Obj*)handle->fiber);
  }

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
  // Explicit cast because size_t has different sizes on 32-bit and 64-bit and
  // we need a consistent type for the format string.
  printf("GC %lu before, %lu after (%lu collected), next at %lu. Took %.3fs.\n",
         (unsigned long)before,
         (unsigned long)vm->bytesAllocated,
         (unsigned long)(before - vm->bytesAllocated),
         (unsigned long)vm->nextGC,
         elapsed);
#endif
}

void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize)
{
#if WREN_DEBUG_TRACE_MEMORY
  // Explicit cast because size_t has different sizes on 32-bit and 64-bit and
  // we need a consistent type for the format string.
  printf("reallocate %p %lu -> %lu\n",
         memory, (unsigned long)oldSize, (unsigned long)newSize);
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
  if (newSize > 0 && vm->bytesAllocated > vm->nextGC) collectGarbage(vm);
#endif

  return vm->reallocate(memory, newSize);
}

// Captures the local variable [local] into an [Upvalue]. If that local is
// already in an upvalue, the existing one will be used. (This is important to
// ensure that multiple closures closing over the same variable actually see
// the same variable.) Otherwise, it will create a new open upvalue and add it
// the fiber's list of upvalues.
static ObjUpvalue* captureUpvalue(WrenVM* vm, ObjFiber* fiber, Value* local)
{
  // If there are no open upvalues at all, we must need a new one.
  if (fiber->openUpvalues == NULL)
  {
    fiber->openUpvalues = wrenNewUpvalue(vm, local);
    return fiber->openUpvalues;
  }

  ObjUpvalue* prevUpvalue = NULL;
  ObjUpvalue* upvalue = fiber->openUpvalues;

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
  ObjUpvalue* createdUpvalue = wrenNewUpvalue(vm, local);
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
  ObjUpvalue* upvalue = fiber->openUpvalues;

  // Move the value into the upvalue itself and point the upvalue to it.
  upvalue->closed = *upvalue->value;
  upvalue->value = &upvalue->closed;

  // Remove it from the open upvalue list.
  fiber->openUpvalues = upvalue->next;
}

// Looks up a foreign method in [moduleName] on [className] with [signature].
//
// This will try the host's foreign method binder first. If that fails, it
// falls back to handling the built-in modules.
static WrenForeignMethodFn findForeignMethod(WrenVM* vm,
                                             const char* moduleName,
                                             const char* className,
                                             bool isStatic,
                                             const char* signature)
{
  WrenForeignMethodFn fn;

  // Let the host try to find it first.
  if (vm->bindForeign != NULL)
  {
    fn = vm->bindForeign(vm, moduleName, className, isStatic, signature);
    if (fn != NULL) return fn;
  }

  // Otherwise, try the built-in libraries.
  if (strcmp(moduleName, "core") == 0)
  {
    #if WREN_USE_LIB_IO
    fn = wrenBindIOForeignMethod(vm, className, signature);
    if (fn != NULL) return fn;
    #endif
  }

  // TODO: Report a runtime error on failure to find it.
  return NULL;
}

// Defines [methodValue] as a method on [classObj].
//
// Handles both foreign methods where [methodValue] is a string containing the
// method's signature and Wren methods where [methodValue] is a function.
//
// Returns an error string if the method is a foreign method that could not be
// found. Otherwise returns `NULL_VAL`.
static Value bindMethod(WrenVM* vm, int methodType, int symbol,
                       ObjModule* module, ObjClass* classObj, Value methodValue)
{
  Method method;
  if (IS_STRING(methodValue))
  {
    const char* name = AS_CSTRING(methodValue);
    method.type = METHOD_FOREIGN;
    method.fn.foreign = findForeignMethod(vm, module->name->value,
                                          classObj->name->value,
                                          methodType == CODE_METHOD_STATIC,
                                          name);

    if (method.fn.foreign == NULL)
    {
      return wrenStringFormat(vm,
          "Could not find foreign method '@' for class $ in module '$'.",
          methodValue, classObj->name->value, module->name->value);
    }
  }
  else
  {
    ObjFn* methodFn = IS_FN(methodValue) ? AS_FN(methodValue)
                                         : AS_CLOSURE(methodValue)->fn;

    // Methods are always bound against the class, and not the metaclass, even
    // for static methods, so that constructors (which are static) get bound
    // like instance methods.
    wrenBindMethodCode(classObj, methodFn);

    method.type = METHOD_BLOCK;
    method.fn.obj = AS_OBJ(methodValue);
  }

  if (methodType == CODE_METHOD_STATIC) classObj = classObj->obj.classObj;
  wrenBindMethod(vm, classObj, symbol, method);
  return NULL_VAL;
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
static ObjFiber* runtimeError(ObjFiber* fiber, Value error)
{
  ASSERT(fiber->error == NULL, "Can only fail once.");

  // Store the error in the fiber so it can be accessed later.
  fiber->error = AS_STRING(error);

  // If the caller ran this fiber using "try", give it the error.
  if (fiber->callerIsTrying)
  {
    ObjFiber* caller = fiber->caller;

    // Make the caller's try method return the error message.
    *(caller->stackTop - 1) = OBJ_VAL(fiber->error);
    return caller;
  }

  // If we got here, nothing caught the error, so show the stack trace.
  wrenDebugPrintStackTrace(fiber);
  return NULL;
}

// Creates a string containing an appropriate method not found error for a
// method with [symbol] on [classObj].
static Value methodNotFound(WrenVM* vm, ObjClass* classObj, int symbol)
{
  return wrenStringFormat(vm, "@ does not implement '$'.",
      OBJ_VAL(classObj->name), vm->methodNames.data[symbol].buffer);
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

// Looks up the core module in the module map.
static ObjModule* getCoreModule(WrenVM* vm)
{
  Value moduleValue = wrenMapGet(vm->modules, NULL_VAL);
  ASSERT(!IS_UNDEFINED(moduleValue), "Could not find core module.");
  return AS_MODULE(moduleValue);
}

static ObjFiber* loadModule(WrenVM* vm, Value name, const char* source)
{
  ObjModule* module;
  Value moduleValue = wrenMapGet(vm->modules, name);

  // See if the module has already been loaded.
  if (!IS_UNDEFINED(moduleValue))
  {
    // Execute the new code in the context of the existing module.
    module = AS_MODULE(moduleValue);
  }
  else
  {
    module = wrenNewModule(vm, AS_STRING(name));

    // Store it in the VM's module registry so we don't load the same module
    // multiple times.
    wrenMapSet(vm, vm->modules, name, OBJ_VAL(module));

    // Implicitly import the core module.
    ObjModule* coreModule = getCoreModule(vm);
    for (int i = 0; i < coreModule->variables.count; i++)
    {
      wrenDefineVariable(vm, module,
                         coreModule->variableNames.data[i].buffer,
                         coreModule->variableNames.data[i].length,
                         coreModule->variables.data[i]);
    }
  }

  ObjFn* fn = wrenCompile(vm, module, AS_CSTRING(name), source, true);
  if (fn == NULL)
  {
    // TODO: Should we still store the module even if it didn't compile?
    return NULL;
  }

  wrenPushRoot(vm, (Obj*)fn);

  ObjFiber* moduleFiber = wrenNewFiber(vm, (Obj*)fn);

  wrenPopRoot(vm); // fn.

  // Return the fiber that executes the module.
  return moduleFiber;
}

static Value importModule(WrenVM* vm, Value name)
{
  // If the module is already loaded, we don't need to do anything.
  if (!IS_UNDEFINED(wrenMapGet(vm->modules, name))) return NULL_VAL;

  // Load the module's source code from the embedder.
  char* source = vm->loadModule(vm, AS_CSTRING(name));
  if (source == NULL)
  {
    // Couldn't load the module.
    return wrenStringFormat(vm, "Could not find module '@'.", name);
  }

  ObjFiber* moduleFiber = loadModule(vm, name, source);

  // Return the fiber that executes the module.
  return OBJ_VAL(moduleFiber);
}


static bool importVariable(WrenVM* vm, Value moduleName, Value variableName,
                            Value* result)
{
  Value moduleValue = wrenMapGet(vm->modules, moduleName);
  ASSERT(!IS_UNDEFINED(moduleValue), "Should only look up loaded modules.");

  ObjModule* module = AS_MODULE(moduleValue);

  ObjString* variable = AS_STRING(variableName);
  uint32_t variableEntry = wrenSymbolTableFind(&module->variableNames,
                                               variable->value,
                                               variable->length);

  // It's a runtime error if the imported variable does not exist.
  if (variableEntry != UINT32_MAX)
  {
    *result = module->variables.data[variableEntry];
    return true;
  }

  *result = wrenStringFormat(vm,
      "Could not find a variable named '@' in module '@'.",
      variableName, moduleName);
  return false;
}

// Verifies that [superclass] is a valid object to inherit from. That means it
// must be a class and cannot be the class of any built-in type.
//
// If successful, returns null. Otherwise, returns a string for the runtime
// error message.
static Value validateSuperclass(WrenVM* vm, Value name,
                                     Value superclassValue)
{
  // Make sure the superclass is a class.
  if (!IS_CLASS(superclassValue))
  {
    return CONST_STRING(vm, "Must inherit from a class.");
  }

  // Make sure it doesn't inherit from a sealed built-in type. Primitive methods
  // on these classes assume the instance is one of the other Obj___ types and
  // will fail horribly if it's actually an ObjInstance.
  ObjClass* superclass = AS_CLASS(superclassValue);
  if (superclass == vm->classClass ||
      superclass == vm->fiberClass ||
      superclass == vm->fnClass || // Includes OBJ_CLOSURE.
      superclass == vm->listClass ||
      superclass == vm->mapClass ||
      superclass == vm->rangeClass ||
      superclass == vm->stringClass)
  {
    return wrenStringFormat(vm, "@ cannot inherit from @.",
        name, OBJ_VAL(superclass->name));
  }

  return NULL_VAL;
}

// Returns `true` if [value] is an instance of [baseClass] or any of its
// superclasses.
static bool isInstanceOf(WrenVM* vm, Value value, ObjClass *baseClass)
{
  ObjClass* classObj = wrenGetClass(vm, value);

  // Walk the superclass chain looking for the class.
  do
  {
    if (classObj == baseClass) return true;

    classObj = classObj->superclass;
  }
  while (classObj != NULL);

  return false;
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
  #define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

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
      do                                               \
      {                                                \
        STORE_FRAME();                                 \
        fiber = runtimeError(fiber, error);            \
        if (fiber == NULL) return false;               \
        LOAD_FRAME();                                  \
        DISPATCH();                                    \
      }                                                \
      while (false)

  #if WREN_DEBUG_TRACE_INSTRUCTIONS
    // Prints the stack and instruction before each instruction is executed.
    #define DEBUG_TRACE_INSTRUCTIONS()                            \
        do                                                        \
        {                                                         \
          wrenDumpStack(fiber);                                   \
          wrenDumpInstruction(vm, fn, (int)(ip - fn->bytecode));  \
        }                                                         \
        while (false)
  #else
    #define DEBUG_TRACE_INSTRUCTIONS() do { } while (false)
  #endif

  #if WREN_COMPUTED_GOTO

  static void* dispatchTable[] = {
    #define OPCODE(name) &&code_##name,
    #include "wren_opcodes.h"
    #undef OPCODE
  };

  #define INTERPRET_LOOP    DISPATCH();
  #define CASE_CODE(name)   code_##name

  #define DISPATCH()                                            \
      do                                                        \
      {                                                         \
        DEBUG_TRACE_INSTRUCTIONS();                             \
        goto *dispatchTable[instruction = (Code)READ_BYTE()];   \
      }                                                         \
      while (false)

  #else

  #define INTERPRET_LOOP                                        \
      loop:                                                     \
        DEBUG_TRACE_INSTRUCTIONS();                             \
        switch (instruction = (Code)READ_BYTE())

  #define CASE_CODE(name)  case CODE_##name
  #define DISPATCH()       goto loop

  #endif

  #define CALL(name)                                                          \
      /* Add one for the implicit receiver argument. */                       \
      int numArgs = instruction - name + 1;                                   \
      int symbol = READ_SHORT();                                              \
                                                                              \
      /* The receiver is the first argument. */                               \
      Value* args = fiber->stackTop - numArgs;                                \
      ObjClass* classObj = wrenGetClassInline(vm, args[0]);                   \
                                                                              \
      if (name == CODE_SUPER_0)                                               \
      {                                                                       \
        /* Ignore methods defined on the receiver's immediate class. */       \
        classObj = classObj->superclass;                                      \
      }                                                                       \
                                                                              \
      /* If the class's method table doesn't include the symbol, bail. */     \
      if (symbol >= classObj->methods.count)                                  \
      {                                                                       \
        RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));                  \
      }                                                                       \
                                                                              \
      Method* method = &classObj->methods.data[symbol];                       \
      switch (method->type)                                                   \
      {                                                                       \
        case METHOD_PRIMITIVE:                                                \
        {                                                                     \
          /* After calling this, the result will be in the first arg slot. */ \
          switch (method->fn.primitive(vm, fiber, args))                      \
          {                                                                   \
            case PRIM_VALUE:                                                  \
              /* The result is now in the first arg slot. Discard the other   \
               stack slots. */                                                \
              fiber->stackTop -= numArgs - 1;                                 \
              break;                                                          \
                                                                              \
            case PRIM_ERROR:                                                  \
              RUNTIME_ERROR(args[0]);                                         \
                                                                              \
            case PRIM_CALL:                                                   \
              STORE_FRAME();                                                  \
              callFunction(fiber, AS_OBJ(args[0]), numArgs);                  \
              LOAD_FRAME();                                                   \
              break;                                                          \
                                                                              \
            case PRIM_RUN_FIBER:                                              \
              STORE_FRAME();                                                  \
                                                                              \
              /* If we don't have a fiber to switch to, stop interpreting. */ \
              if (IS_NULL(args[0])) return true;                              \
                                                                              \
              fiber = AS_FIBER(args[0]);                                      \
              vm->fiber = fiber;                                              \
              LOAD_FRAME();                                                   \
              break;                                                          \
          }                                                                   \
          break;                                                              \
        }                                                                     \
                                                                              \
        case METHOD_FOREIGN:                                                  \
          callForeign(vm, fiber, method->fn.foreign, numArgs);                \
          break;                                                              \
                                                                              \
        case METHOD_BLOCK:                                                    \
          STORE_FRAME();                                                      \
          callFunction(fiber, method->fn.obj, numArgs);                       \
          LOAD_FRAME();                                                       \
          break;                                                              \
                                                                              \
        case METHOD_NONE:                                                     \
          RUNTIME_ERROR(methodNotFound(vm, classObj, symbol));                \
          break;                                                              \
      }                                                                       \
      DISPATCH();

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
      CALL(CODE_CALL_0);
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
      CALL(CODE_SUPER_0);
    }

    CASE_CODE(LOAD_UPVALUE):
    {
      ObjUpvalue** upvalues = ((ObjClosure*)frame->fn)->upvalues;
      PUSH(*upvalues[READ_BYTE()]->value);
      DISPATCH();
    }

    CASE_CODE(STORE_UPVALUE):
    {
      ObjUpvalue** upvalues = ((ObjClosure*)frame->fn)->upvalues;
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
        RUNTIME_ERROR(CONST_STRING(vm, "Right operand must be a class."));
      }

      Value instance = POP();
      PUSH(BOOL_VAL(isInstanceOf(vm, instance, AS_CLASS(expected))));
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
        vm->fiber = fiber;

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
      Value name = PEEK2();
      ObjClass* superclass = vm->objectClass;

      // Use implicit Object superclass if none given.
      if (!IS_NULL(PEEK()))
      {
        Value error = validateSuperclass(vm, name, PEEK());
        if (!IS_NULL(error)) RUNTIME_ERROR(error);
        superclass = AS_CLASS(PEEK());
      }

      int numFields = READ_BYTE();

      Value classObj = OBJ_VAL(wrenNewClass(vm, superclass, numFields,
                                            AS_STRING(name)));

      // Don't pop the superclass and name off the stack until the subclass is
      // done being created, to make sure it doesn't get collected.
      DROP();
      DROP();

      // Now that we know the total number of fields, make sure we don't
      // overflow.
      if (superclass->numFields + numFields > MAX_FIELDS)
      {
        RUNTIME_ERROR(wrenStringFormat(vm,
            "Class '@' may not have more than 255 fields, including inherited "
            "ones.", name));
      }

      PUSH(classObj);
      DISPATCH();
    }

    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      uint16_t symbol = READ_SHORT();
      ObjClass* classObj = AS_CLASS(PEEK());
      Value method = PEEK2();
      Value error = bindMethod(vm, instruction, symbol, fn->module, classObj,
                               method);
      if (IS_STRING(error)) RUNTIME_ERROR(error);
      DROP();
      DROP();
      DISPATCH();
    }

    CASE_CODE(LOAD_MODULE):
    {
      Value name = fn->constants[READ_SHORT()];
      Value result = importModule(vm, name);

      // If it returned a string, it was an error message.
      if (IS_STRING(result)) RUNTIME_ERROR(result);

      // Make a slot that the module's fiber can use to store its result in.
      // It ends up getting discarded, but CODE_RETURN expects to be able to
      // place a value there.
      PUSH(NULL_VAL);

      // If it returned a fiber to execute the module body, switch to it.
      if (IS_FIBER(result))
      {
        // Return to this module when that one is done.
        AS_FIBER(result)->caller = fiber;

        STORE_FRAME();
        fiber = AS_FIBER(result);
        vm->fiber = fiber;
        LOAD_FRAME();
      }

      DISPATCH();
    }

    CASE_CODE(IMPORT_VARIABLE):
    {
      Value module = fn->constants[READ_SHORT()];
      Value variable = fn->constants[READ_SHORT()];
      Value result;
      if (importVariable(vm, module, variable, &result))
      {
        PUSH(result);
      }
      else
      {
        RUNTIME_ERROR(result);
      }
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

// Creates an [ObjFn] that invokes a method with [signature] when called.
static ObjFn* makeCallStub(WrenVM* vm, ObjModule* module, const char* signature)
{
  int signatureLength = (int)strlen(signature);

  // Count the number parameters the method expects.
  int numParams = 0;
  for (const char* s = signature; *s != '\0'; s++)
  {
    if (*s == '_') numParams++;
  }

  int method =  wrenSymbolTableEnsure(vm, &vm->methodNames,
                                      signature, signatureLength);

  uint8_t* bytecode = ALLOCATE_ARRAY(vm, uint8_t, 5);
  bytecode[0] = (uint8_t)(CODE_CALL_0 + numParams);
  bytecode[1] = (method >> 8) & 0xff;
  bytecode[2] = method & 0xff;
  bytecode[3] = CODE_RETURN;
  bytecode[4] = CODE_END;

  int* debugLines = ALLOCATE_ARRAY(vm, int, 5);
  memset(debugLines, 1, 5);

  return wrenNewFunction(vm, module, NULL, 0, 0, 0, bytecode, 5, NULL,
                         signature, signatureLength, debugLines);
}

WrenMethod* wrenGetMethod(WrenVM* vm, const char* module, const char* variable,
                          const char* signature)
{
  Value moduleName = wrenStringFormat(vm, "$", module);
  wrenPushRoot(vm, AS_OBJ(moduleName));

  Value moduleValue = wrenMapGet(vm->modules, moduleName);
  // TODO: Handle module not being found.
  ObjModule* moduleObj = AS_MODULE(moduleValue);

  int variableSlot = wrenSymbolTableFind(&moduleObj->variableNames,
                                         variable, strlen(variable));
  // TODO: Handle the variable not being found.

  ObjFn* fn = makeCallStub(vm, moduleObj, signature);
  wrenPushRoot(vm, (Obj*)fn);

  // Create a single fiber that we can reuse each time the method is invoked.
  ObjFiber* fiber = wrenNewFiber(vm, (Obj*)fn);
  wrenPushRoot(vm, (Obj*)fiber);

  // Create a handle that keeps track of the function that calls the method.
  WrenMethod* method = ALLOCATE(vm, WrenMethod);
  method->fiber = fiber;

  // Store the receiver in the fiber's stack so we can use it later in the call.
  *fiber->stackTop++ = moduleObj->variables.data[variableSlot];

  // Add it to the front of the linked list of handles.
  if (vm->methodHandles != NULL) vm->methodHandles->prev = method;
  method->prev = NULL;
  method->next = vm->methodHandles;
  vm->methodHandles = method;

  wrenPopRoot(vm); // fiber.
  wrenPopRoot(vm); // fn.
  wrenPopRoot(vm); // moduleName.

  return method;
}

void wrenCall(WrenVM* vm, WrenMethod* method, const char* argTypes, ...)
{
  // TODO: Validate that the number of arguments matches what the method
  // expects.

  // Push the arguments.
  va_list argList;
  va_start(argList, argTypes);

  for (const char* argType = argTypes; *argType != '\0'; argType++)
  {
    Value value = NULL_VAL;
    switch (*argType)
    {
      case 'b': value = BOOL_VAL(va_arg(argList, int)); break;
      case 'd': value = NUM_VAL(va_arg(argList, double)); break;
      case 'i': value = NUM_VAL((double)va_arg(argList, int)); break;
      case 'n': value = NULL_VAL; va_arg(argList, void*); break;
      case 's':
      {
        value = wrenStringFormat(vm, "$", va_arg(argList, const char*));
        break;
      }

      default:
        ASSERT(false, "Unknown argument type.");
        break;
    }

    *method->fiber->stackTop++ = value;
  }

  va_end(argList);

  vm->fiber = method->fiber;

  Value receiver = method->fiber->stack[0];
  Obj* fn = method->fiber->frames[0].fn;

  // TODO: How does this handle a runtime error occurring?
  runInterpreter(vm);

  // Reset the fiber to get ready for the next call.
  wrenResetFiber(method->fiber, fn);

  // Push the receiver back on the stack.
  *method->fiber->stackTop++ = receiver;
}

void wrenReleaseMethod(WrenVM* vm, WrenMethod* method)
{
  ASSERT(method != NULL, "NULL method.");

  // Update the VM's head pointer if we're releasing the first handle.
  if (vm->methodHandles == method) vm->methodHandles = method->next;

  // Unlink it from the list.
  if (method->prev != NULL) method->prev->next = method->next;
  if (method->next != NULL) method->next->prev = method->prev;

  // Clear it out. This isn't strictly necessary since we're going to free it,
  // but it makes for easier debugging.
  method->prev = NULL;
  method->next = NULL;
  method->fiber = NULL;
  DEALLOCATE(vm, method);
}

// Execute [source] in the context of the core module.
static WrenInterpretResult loadIntoCore(WrenVM* vm, const char* source)
{
  ObjModule* coreModule = getCoreModule(vm);

  ObjFn* fn = wrenCompile(vm, coreModule, "", source, true);
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
  Value name = CONST_STRING(vm, "main");
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
  Value nameValue = wrenStringFormat(vm, "$", name);
  wrenPushRoot(vm, AS_OBJ(nameValue));

  // If the module is already loaded, we don't need to do anything.
  if (!IS_UNDEFINED(wrenMapGet(vm->modules, nameValue)))
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
    return wrenStringFormat(vm, "Could not find module '$'.", name);
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
