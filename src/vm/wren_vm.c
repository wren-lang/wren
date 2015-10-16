#include <stdarg.h>
#include <string.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_core.h"
#include "wren_debug.h"
#include "wren_vm.h"

#if WREN_USE_META_MODULE
  #include "wren_meta.h"
#endif

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
  #include <time.h>
#endif

// The behavior of realloc() when the size is 0 is implementation defined. It
// may return a non-NULL pointer which must not be dereferenced but nevertheless
// should be freed. To prevent that, we avoid calling realloc() with a zero
// size.
static void* defaultReallocate(void* ptr, size_t newSize)
{
  if (newSize == 0)
  {
    free(ptr);
    return NULL;
  }

  return realloc(ptr, newSize);
}

void wrenInitConfiguration(WrenConfiguration* config)
{
  config->reallocateFn = defaultReallocate;
  config->loadModuleFn = NULL;
  config->bindForeignMethodFn = NULL;
  config->bindForeignClassFn = NULL;
  config->writeFn = NULL;
  config->initialHeapSize = 1024 * 1024 * 10;
  config->minHeapSize = 1024 * 1024;
  config->heapGrowthPercent = 50;
}

WrenVM* wrenNewVM(WrenConfiguration* config)
{
  WrenVM* vm = (WrenVM*)config->reallocateFn(NULL, sizeof(*vm));
  memset(vm, 0, sizeof(WrenVM));
  memcpy(&vm->config, config, sizeof(WrenConfiguration));

  vm->nextGC = config->initialHeapSize;

  wrenSymbolTableInit(&vm->methodNames);

  vm->modules = wrenNewMap(vm);

  wrenInitializeCore(vm);
  
  #if WREN_USE_META_MODULE
    wrenLoadMetaModule(vm);
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

  // Tell the user if they didn't free any handles. We don't want to just free
  // them here because the host app may still have pointers to them that they
  // may try to use. Better to tell them about the bug early.
  ASSERT(vm->valueHandles == NULL, "All values have not been released.");
  
  wrenSymbolTableClear(vm, &vm->methodNames);

  DEALLOCATE(vm, vm);
}

void wrenSetCompiler(WrenVM* vm, Compiler* compiler)
{
  vm->compiler = compiler;
}

void wrenCollectGarbage(WrenVM* vm)
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

  wrenMarkObj(vm, (Obj*)vm->modules);

  // Temporary roots.
  for (int i = 0; i < vm->numTempRoots; i++)
  {
    wrenMarkObj(vm, vm->tempRoots[i]);
  }

  // The current fiber.
  wrenMarkObj(vm, (Obj*)vm->fiber);

  // The value handles.
  for (WrenValue* value = vm->valueHandles;
       value != NULL;
       value = value->next)
  {
    wrenMarkValue(vm, value->value);
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

  // +100 here because the configuration gives us the *additional* size of
  // the heap relative to the in-use memory, while heapScalePercent is the
  // *total* size of the heap relative to in-use.
  vm->nextGC = vm->bytesAllocated * (100 + vm->config.heapGrowthPercent) / 100;
  if (vm->nextGC < vm->config.minHeapSize) vm->nextGC = vm->config.minHeapSize;

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
  if (newSize > 0) wrenCollectGarbage(vm);
#else
  if (newSize > 0 && vm->bytesAllocated > vm->nextGC) wrenCollectGarbage(vm);
#endif

  return vm->config.reallocateFn(memory, newSize);
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

// Closes any open upvates that have been created for stack slots at [last] and
// above.
static void closeUpvalues(ObjFiber* fiber, Value* last)
{
  while (fiber->openUpvalues != NULL &&
         fiber->openUpvalues->value >= last)
  {
    ObjUpvalue* upvalue = fiber->openUpvalues;
    
    // Move the value into the upvalue itself and point the upvalue to it.
    upvalue->closed = *upvalue->value;
    upvalue->value = &upvalue->closed;
    
    // Remove it from the open upvalue list.
    fiber->openUpvalues = upvalue->next;
  }
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
  if (vm->config.bindForeignMethodFn == NULL) return NULL;

  return vm->config.bindForeignMethodFn(vm, moduleName, className, isStatic, signature);
}

// Defines [methodValue] as a method on [classObj].
//
// Handles both foreign methods where [methodValue] is a string containing the
// method's signature and Wren methods where [methodValue] is a function.
//
// Aborts the current fiber if the method is a foreign method that could not be
// found.
static void bindMethod(WrenVM* vm, int methodType, int symbol,
                       ObjModule* module, ObjClass* classObj, Value methodValue)
{
  const char* className = classObj->name->value;
  if (methodType == CODE_METHOD_STATIC) classObj = classObj->obj.classObj;

  Method method;
  if (IS_STRING(methodValue))
  {
    const char* name = AS_CSTRING(methodValue);
    method.type = METHOD_FOREIGN;
    method.fn.foreign = findForeignMethod(vm, module->name->value,
                                          className,
                                          methodType == CODE_METHOD_STATIC,
                                          name);

    if (method.fn.foreign == NULL)
    {
      vm->fiber->error = wrenStringFormat(vm,
          "Could not find foreign method '@' for class $ in module '$'.",
          methodValue, classObj->name->value, module->name->value);
      return;
    }
  }
  else
  {
    ObjFn* methodFn = IS_FN(methodValue) ? AS_FN(methodValue)
                                         : AS_CLOSURE(methodValue)->fn;

    // Patch up the bytecode now that we know the superclass.
    wrenBindMethodCode(classObj, methodFn);

    method.type = METHOD_BLOCK;
    method.fn.obj = AS_OBJ(methodValue);
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

// Handles the current fiber having aborted because of an error. Switches to
// a new fiber if there is a fiber that will handle the error, otherwise, tells
// the VM to stop.
static void runtimeError(WrenVM* vm)
{
  ASSERT(!IS_NULL(vm->fiber->error), "Should only call this after an error.");
  
  // Unhook the caller since we will never resume and return to it.
  ObjFiber* caller = vm->fiber->caller;
  vm->fiber->caller = NULL;
  
  // If the caller ran this fiber using "try", give it the error.
  if (vm->fiber->callerIsTrying)
  {
    // Make the caller's try method return the error message.
    caller->stackTop[-1] = vm->fiber->error;
    
    vm->fiber = caller;
    return;
  }
  
  // If we got here, nothing caught the error, so show the stack trace.
  wrenDebugPrintStackTrace(vm->fiber);
  vm->fiber = NULL;
}

// Aborts the current fiber with an appropriate method not found error for a
// method with [symbol] on [classObj].
static void methodNotFound(WrenVM* vm, ObjClass* classObj, int symbol)
{
  vm->fiber->error = wrenStringFormat(vm, "@ does not implement '$'.",
      OBJ_VAL(classObj->name), vm->methodNames.data[symbol].buffer);
}

// Checks that [value], which must be a function or closure, does not require
// more parameters than are provided by [numArgs].
//
// If there are not enough arguments, aborts the current fiber and returns
// `false`.
static bool checkArity(WrenVM* vm, Value value, int numArgs)
{
  ObjFn* fn;
  if (IS_CLOSURE(value))
  {
    fn = AS_CLOSURE(value)->fn;
  }
  else
  {
    ASSERT(IS_FN(value), "Receiver must be a function or closure.");
    fn = AS_FN(value);
  }
  
  // We only care about missing arguments, not extras. The "- 1" is because
  // numArgs includes the receiver, the function itself, which we don't want to
  // count.
  if (numArgs - 1 >= fn->arity) return true;
  
  vm->fiber->error = CONST_STRING(vm, "Function expects more arguments.");
  return false;
}

// Pushes [function] onto [fiber]'s callstack and invokes it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
// [function] can be an `ObjFn` or `ObjClosure`.
static inline void callFunction(
    WrenVM* vm, ObjFiber* fiber, Obj* function, int numArgs)
{
  if (fiber->numFrames + 1 > fiber->frameCapacity)
  {
    int max = fiber->frameCapacity * 2;
    fiber->frames = (CallFrame*)wrenReallocate(vm, fiber->frames,
        sizeof(CallFrame) * fiber->frameCapacity,
        sizeof(CallFrame) * max);
    fiber->frameCapacity = max;
  }
  
  // TODO: Check for stack overflow. We handle the call frame array growing,
  // but not the stack itself.

  wrenAppendCallFrame(vm, fiber, function, fiber->stackTop - numArgs);
}

// Looks up the previously loaded module with [name].
//
// Returns `NULL` if no module with that name has been loaded.
static ObjModule* getModule(WrenVM* vm, Value name)
{
  Value moduleValue = wrenMapGet(vm->modules, name);
  return !IS_UNDEFINED(moduleValue) ? AS_MODULE(moduleValue) : NULL;
}

static ObjFiber* loadModule(WrenVM* vm, Value name, const char* source)
{
  ObjModule* module = getModule(vm, name);

  // See if the module has already been loaded.
  if (module == NULL)
  {
    module = wrenNewModule(vm, AS_STRING(name));

    // Store it in the VM's module registry so we don't load the same module
    // multiple times.
    wrenMapSet(vm, vm->modules, name, OBJ_VAL(module));

    // Implicitly import the core module (unless we *are* core).
    if (!IS_NULL(name))
    {
      ObjModule* coreModule = getModule(vm, NULL_VAL);
      for (int i = 0; i < coreModule->variables.count; i++)
      {
        wrenDefineVariable(vm, module,
                           coreModule->variableNames.data[i].buffer,
                           coreModule->variableNames.data[i].length,
                           coreModule->variables.data[i]);
      }
    }
  }

  ObjFn* fn = wrenCompile(vm, module, source, true);
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
  char* source = vm->config.loadModuleFn(vm, AS_CSTRING(name));
  if (source == NULL)
  {
    // Couldn't load the module.
    vm->fiber->error = wrenStringFormat(vm, "Could not find module '@'.", name);
    return NULL_VAL;
  }

  ObjFiber* moduleFiber = loadModule(vm, name, source);

  // Return the fiber that executes the module.
  return OBJ_VAL(moduleFiber);
}

static Value importVariable(WrenVM* vm, Value moduleName, Value variableName)
{
  ObjModule* module = getModule(vm, moduleName);
  ASSERT(module != NULL, "Should only look up loaded modules.");

  ObjString* variable = AS_STRING(variableName);
  uint32_t variableEntry = wrenSymbolTableFind(&module->variableNames,
                                               variable->value,
                                               variable->length);

  // It's a runtime error if the imported variable does not exist.
  if (variableEntry != UINT32_MAX)
  {
    return module->variables.data[variableEntry];
  }

  vm->fiber->error = wrenStringFormat(vm,
      "Could not find a variable named '@' in module '@'.",
      variableName, moduleName);
  return NULL_VAL;
}

// Verifies that [superclassValue] is a valid object to inherit from. That
// means it must be a class and cannot be the class of any built-in type.
//
// Also validates that it doesn't result in a class with too many fields and
// the other limitations foreign classes have.
//
// If successful, returns `null`. Otherwise, returns a string for the runtime
// error message.
static Value validateSuperclass(WrenVM* vm, Value name, Value superclassValue,
                                int numFields)
{
  // Make sure the superclass is a class.
  if (!IS_CLASS(superclassValue))
  {
    return wrenStringFormat(vm,
        "Class '@' cannot inherit from a non-class object.",
        name);
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
    return wrenStringFormat(vm,
        "Class '@' cannot inherit from built-in class '@'.",
        name, OBJ_VAL(superclass->name));
  }
  
  if (superclass->numFields == -1)
  {
    return wrenStringFormat(vm,
        "Class '@' cannot inherit from foreign class '@'.",
        name, OBJ_VAL(superclass->name));
  }

  if (numFields == -1 && superclass->numFields > 0)
  {
    return wrenStringFormat(vm,
        "Foreign class '@' may not inherit from a class with fields.",
        name);
  }
  
  if (superclass->numFields + numFields > MAX_FIELDS)
  {
    return wrenStringFormat(vm,
        "Class '@' may not have more than 255 fields, including inherited "
        "ones.", name);
  }

  return NULL_VAL;
}

static void bindForeignClass(WrenVM* vm, ObjClass* classObj, ObjModule* module)
{
  // TODO: Make this a runtime error?
  ASSERT(vm->config.bindForeignClassFn != NULL,
      "Cannot declare foreign classes without a bindForeignClassFn.");
  
  WrenForeignClassMethods methods = vm->config.bindForeignClassFn(
      vm, module->name->value, classObj->name->value);
  
  Method method;
  method.type = METHOD_FOREIGN;
  method.fn.foreign = methods.allocate;
  
  ASSERT(method.fn.foreign != NULL,
      "A foreign class must provide an allocate function.");
  
  int symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, "<allocate>", 10);
  wrenBindMethod(vm, classObj, symbol, method);
  
  // Add the symbol even if there is no finalizer so we can ensure that the
  // symbol itself is always in the symbol table.
  symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, "<finalize>", 10);
  
  if (methods.finalize != NULL)
  {
    method.fn.foreign = methods.finalize;
    wrenBindMethod(vm, classObj, symbol, method);
  }
}

// Creates a new class.
//
// If [numFields] is -1, the class is a foreign class. The name and superclass
// should be on top of the fiber's stack. After calling this, the top of the
// stack will contain the new class.
//
// Aborts the current fiber if an error occurs.
static void createClass(WrenVM* vm, int numFields, ObjModule* module)
{
  // Pull the name and superclass off the stack.
  Value name = vm->fiber->stackTop[-2];
  Value superclass = vm->fiber->stackTop[-1];
  
  // We have two values on the stack and we are going to leave one, so discard
  // the other slot.
  vm->fiber->stackTop--;

  vm->fiber->error = validateSuperclass(vm, name, superclass, numFields);
  if (!IS_NULL(vm->fiber->error)) return;
  
  ObjClass* classObj = wrenNewClass(vm, AS_CLASS(superclass), numFields,
                                    AS_STRING(name));
  vm->fiber->stackTop[-1] = OBJ_VAL(classObj);
  
  if (numFields == -1) bindForeignClass(vm, classObj, module);
}

static void createForeign(WrenVM* vm, ObjFiber* fiber, Value* stack)
{
  ObjClass* classObj = AS_CLASS(stack[0]);
  ASSERT(classObj->numFields == -1, "Class must be a foreign class.");
  
  // TODO: Don't look up every time.
  int symbol = wrenSymbolTableFind(&vm->methodNames, "<allocate>", 10);
  ASSERT(symbol != -1, "Should have defined <allocate> symbol.");
  
  ASSERT(classObj->methods.count > symbol, "Class should have allocator.");
  Method* method = &classObj->methods.data[symbol];
  ASSERT(method->type == METHOD_FOREIGN, "Allocator should be foreign.");
  
  // Pass the constructor arguments to the allocator as well.
  vm->foreignCallSlot = stack;
  vm->foreignCallNumArgs = (int)(fiber->stackTop - stack);
  
  method->fn.foreign(vm);
  
  // TODO: Check that allocateForeign was called.
}

void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign)
{
  // TODO: Don't look up every time.
  int symbol = wrenSymbolTableFind(&vm->methodNames, "<finalize>", 10);
  ASSERT(symbol != -1, "Should have defined <finalize> symbol.");
  
  // If there are no finalizers, don't finalize it.
  if (symbol == -1) return;
  
  // If the class doesn't have a finalizer, bail out.
  ObjClass* classObj = foreign->obj.classObj;
  if (symbol >= classObj->methods.count) return;
  
  Method* method = &classObj->methods.data[symbol];
  if (method->type == METHOD_NONE) return;
  
  ASSERT(method->type == METHOD_FOREIGN, "Finalizer should be foreign.");
  
  // Pass the constructor arguments to the allocator as well.
  Value slot = OBJ_VAL(foreign);
  vm->foreignCallSlot = &slot;
  vm->foreignCallNumArgs = 1;
  
  method->fn.foreign(vm);
}

// The main bytecode interpreter loop. This is where the magic happens. It is
// also, as you can imagine, highly performance critical. Returns `true` if the
// fiber completed without error.
static WrenInterpretResult runInterpreter(WrenVM* vm, register ObjFiber* fiber)
{
  // Remember the current fiber so we can find it if a GC happens.
  vm->fiber = fiber;

  // Hoist these into local variables. They are accessed frequently in the loop
  // but assigned less frequently. Keeping them in locals and updating them when
  // a call frame has been pushed or popped gives a large speed boost.
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
      fn = wrenGetFrameFunction(frame);

  // Terminates the current fiber with error string [error]. If another calling
  // fiber is willing to catch the error, transfers control to it, otherwise
  // exits the interpreter.
  #define RUNTIME_ERROR()                                         \
      do                                                          \
      {                                                           \
        STORE_FRAME();                                            \
        runtimeError(vm);                                         \
        if (vm->fiber == NULL) return WREN_RESULT_RUNTIME_ERROR;  \
        fiber = vm->fiber;                                        \
        LOAD_FRAME();                                             \
        DISPATCH();                                               \
      }                                                           \
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

    CASE_CODE(STORE_LOCAL):
      stackStart[READ_BYTE()] = PEEK();
      DISPATCH();

    CASE_CODE(CONSTANT):
      PUSH(fn->constants[READ_SHORT()]);
      DISPATCH();

    {
      // The opcodes for doing method and superclass calls share a lot of code.
      // However, doing an if() test in the middle of the instruction sequence
      // to handle the bit that is special to super calls makes the non-super
      // call path noticeably slower.
      //
      // Instead, we do this old school using an explicit goto to share code for
      // everything at the tail end of the call-handling code that is the same
      // between normal and superclass calls.
      int numArgs;
      int symbol;

      Value* args;
      ObjClass* classObj;

      Method* method;

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
      // Add one for the implicit receiver argument.
      numArgs = instruction - CODE_CALL_0 + 1;
      symbol = READ_SHORT();

      // The receiver is the first argument.
      args = fiber->stackTop - numArgs;
      classObj = wrenGetClassInline(vm, args[0]);
      goto completeCall;

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
      // Add one for the implicit receiver argument.
      numArgs = instruction - CODE_SUPER_0 + 1;
      symbol = READ_SHORT();

      // The receiver is the first argument.
      args = fiber->stackTop - numArgs;

      // The superclass is stored in a constant.
      classObj = AS_CLASS(fn->constants[READ_SHORT()]);
      goto completeCall;

    completeCall:
      // If the class's method table doesn't include the symbol, bail.
      if (symbol >= classObj->methods.count ||
          (method = &classObj->methods.data[symbol])->type == METHOD_NONE)
      {
        methodNotFound(vm, classObj, symbol);
        RUNTIME_ERROR();
      }

      switch (method->type)
      {
        case METHOD_PRIMITIVE:
          if (method->fn.primitive(vm, args))
          {
            // The result is now in the first arg slot. Discard the other
            // stack slots.
            fiber->stackTop -= numArgs - 1;
          } else {
            // An error or fiber switch occurred.
            STORE_FRAME();

            // If we don't have a fiber to switch to, stop interpreting.
            fiber = vm->fiber;
            if (fiber == NULL) return WREN_RESULT_SUCCESS;
            if (!IS_NULL(fiber->error)) RUNTIME_ERROR();
            LOAD_FRAME();
          }
          break;

        case METHOD_FOREIGN:
          callForeign(vm, fiber, method->fn.foreign, numArgs);
          break;
          
        case METHOD_FN_CALL:
          if (!checkArity(vm, args[0], numArgs)) RUNTIME_ERROR();

          STORE_FRAME();
          callFunction(vm, fiber, AS_OBJ(args[0]), numArgs);
          LOAD_FRAME();
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(vm, fiber, method->fn.obj, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          UNREACHABLE();
      }
      DISPATCH();
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

    CASE_CODE(CLOSE_UPVALUE):
      // Close the upvalue for the local if we have one.
      closeUpvalues(fiber, fiber->stackTop - 1);
      DROP();
      DISPATCH();

    CASE_CODE(RETURN):
    {
      Value result = POP();
      fiber->numFrames--;

      // Close any upvalues still in scope.
      closeUpvalues(fiber, stackStart);
      
      // If the fiber is complete, end it.
      if (fiber->numFrames == 0)
      {
        // If this is the main fiber, we're done.
        if (fiber->caller == NULL) return WREN_RESULT_SUCCESS;

        // We have a calling fiber to resume.
        ObjFiber* callingFiber = fiber->caller;
        fiber->caller = NULL;
        
        fiber = callingFiber;
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

    CASE_CODE(CONSTRUCT):
      ASSERT(IS_CLASS(stackStart[0]), "'this' should be a class.");
      stackStart[0] = wrenNewInstance(vm, AS_CLASS(stackStart[0]));
      DISPATCH();
    
    CASE_CODE(FOREIGN_CONSTRUCT):
      ASSERT(IS_CLASS(stackStart[0]), "'this' should be a class.");
      createForeign(vm, fiber, stackStart);
      DISPATCH();

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
      createClass(vm, READ_BYTE(), NULL);
      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();
      DISPATCH();
    }
    
    CASE_CODE(FOREIGN_CLASS):
    {
      createClass(vm, -1, fn->module);
      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();
      DISPATCH();
    }
    
    CASE_CODE(METHOD_INSTANCE):
    CASE_CODE(METHOD_STATIC):
    {
      uint16_t symbol = READ_SHORT();
      ObjClass* classObj = AS_CLASS(PEEK());
      Value method = PEEK2();
      bindMethod(vm, instruction, symbol, fn->module, classObj, method);
      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();
      DROP();
      DROP();
      DISPATCH();
    }

    CASE_CODE(LOAD_MODULE):
    {
      Value name = fn->constants[READ_SHORT()];
      Value result = importModule(vm, name);

      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();

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
      Value result = importVariable(vm, module, variable);
      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();

      PUSH(result);
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

  #undef READ_BYTE
  #undef READ_SHORT
}

// Creates an [ObjFn] that invokes a method with [signature] when called.
static ObjFn* makeCallStub(WrenVM* vm, ObjModule* module, const char* signature)
{
  int signatureLength = (int)strlen(signature);

  // Count the number parameters the method expects.
  int numParams = 0;
  if (signature[signatureLength - 1] == ')')
  {
    for (const char* s = signature + signatureLength - 2;
         s > signature && *s != '('; s--)
    {
      if (*s == '_') numParams++;
    }
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

  return wrenNewFunction(vm, module, NULL, 0, 0, 0, bytecode, 5,
                         signature, signatureLength, debugLines);
}

WrenValue* wrenGetMethod(WrenVM* vm, const char* module, const char* variable,
                         const char* signature)
{
  Value moduleName = wrenStringFormat(vm, "$", module);
  wrenPushRoot(vm, AS_OBJ(moduleName));

  ObjModule* moduleObj = getModule(vm, moduleName);
  // TODO: Handle module not being found.

  int variableSlot = wrenSymbolTableFind(&moduleObj->variableNames,
                                         variable, strlen(variable));
  // TODO: Handle the variable not being found.

  ObjFn* fn = makeCallStub(vm, moduleObj, signature);
  wrenPushRoot(vm, (Obj*)fn);

  // Create a single fiber that we can reuse each time the method is invoked.
  ObjFiber* fiber = wrenNewFiber(vm, (Obj*)fn);
  wrenPushRoot(vm, (Obj*)fiber);

  // Create a handle that keeps track of the function that calls the method.
  WrenValue* method = wrenCaptureValue(vm, OBJ_VAL(fiber));

  // Store the receiver in the fiber's stack so we can use it later in the call.
  *fiber->stackTop++ = moduleObj->variables.data[variableSlot];

  wrenPopRoot(vm); // fiber.
  wrenPopRoot(vm); // fn.
  wrenPopRoot(vm); // moduleName.

  return method;
}

WrenInterpretResult wrenCall(WrenVM* vm, WrenValue* method,
                             WrenValue** returnValue,
                             const char* argTypes, ...)
{
  va_list args;
  va_start(args, argTypes);
  WrenInterpretResult result = wrenCallVarArgs(vm, method, returnValue,
                                               argTypes, args);
  va_end(args);
  
  return result;
}

WrenInterpretResult wrenCallVarArgs(WrenVM* vm, WrenValue* method,
                                    WrenValue** returnValue,
                                    const char* argTypes, va_list args)
{
  // TODO: Validate that the number of arguments matches what the method
  // expects.
  
  ASSERT(IS_FIBER(method->value), "Value must come from wrenGetMethod().");
  ObjFiber* fiber = AS_FIBER(method->value);
  
  // Push the arguments.
  for (const char* argType = argTypes; *argType != '\0'; argType++)
  {
    Value value = NULL_VAL;
    switch (*argType)
    {
      case 'a':
      {
        const char* bytes = va_arg(args, const char*);
        int length = va_arg(args, int);
        value = wrenNewString(vm, bytes, (size_t)length);
        break;
      }
        
      case 'b': value = BOOL_VAL(va_arg(args, int)); break;
      case 'd': value = NUM_VAL(va_arg(args, double)); break;
      case 'i': value = NUM_VAL((double)va_arg(args, int)); break;
      case 'n': value = NULL_VAL; va_arg(args, void*); break;
      case 's':
        value = wrenStringFormat(vm, "$", va_arg(args, const char*));
        break;
        
      case 'v':
      {
        // Allow a NULL value pointer for Wren null.
        WrenValue* wrenValue = va_arg(args, WrenValue*);
        if (wrenValue != NULL) value = wrenValue->value;
        break;
      }
        
      default:
        ASSERT(false, "Unknown argument type.");
        break;
    }
    
    *fiber->stackTop++ = value;
  }
  
  Value receiver = fiber->stack[0];
  Obj* fn = fiber->frames[0].fn;
  wrenPushRoot(vm, (Obj*)fn);
  
  WrenInterpretResult result = runInterpreter(vm, fiber);
  
  if (result == WREN_RESULT_SUCCESS)
  {
    if (returnValue != NULL)
    {
      // Make sure the return value doesn't get collected while capturing it.
      fiber->stackTop++;
      *returnValue = wrenCaptureValue(vm, fiber->stack[0]);
    }
    
    // Reset the fiber to get ready for the next call.
    wrenResetFiber(vm, fiber, fn);
    
    // Push the receiver back on the stack.
    *fiber->stackTop++ = receiver;
  }
  else
  {
    if (returnValue != NULL) *returnValue = NULL;
  }
  
  wrenPopRoot(vm);
  
  return result;
}

WrenValue* wrenCaptureValue(WrenVM* vm, Value value)
{
  // Make a handle for it.
  WrenValue* wrappedValue = ALLOCATE(vm, WrenValue);
  wrappedValue->value = value;

  // Add it to the front of the linked list of handles.
  if (vm->valueHandles != NULL) vm->valueHandles->prev = wrappedValue;
  wrappedValue->prev = NULL;
  wrappedValue->next = vm->valueHandles;
  vm->valueHandles = wrappedValue;

  return wrappedValue;
}

void wrenReleaseValue(WrenVM* vm, WrenValue* value)
{
  ASSERT(value != NULL, "NULL value.");
  
  // Update the VM's head pointer if we're releasing the first handle.
  if (vm->valueHandles == value) vm->valueHandles = value->next;
  
  // Unlink it from the list.
  if (value->prev != NULL) value->prev->next = value->next;
  if (value->next != NULL) value->next->prev = value->prev;
  
  // Clear it out. This isn't strictly necessary since we're going to free it,
  // but it makes for easier debugging.
  value->prev = NULL;
  value->next = NULL;
  value->value = NULL_VAL;
  DEALLOCATE(vm, value);
}

void* wrenAllocateForeign(WrenVM* vm, size_t size)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");

  // TODO: Validate this. It can fail if the user calls this inside another
  // foreign method, or calls one of the return functions.
  ObjClass* classObj = AS_CLASS(vm->foreignCallSlot[0]);

  ObjForeign* foreign = wrenNewForeign(vm, classObj, size);
  vm->foreignCallSlot[0] = OBJ_VAL(foreign);
  
  return (void*)foreign->data;
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* source)
{
  return wrenInterpretInModule(vm, "main", source);
}

WrenInterpretResult wrenInterpretInModule(WrenVM* vm, const char* module,
                                          const char* source)
{
  Value nameValue = NULL_VAL;
  if (module != NULL)
  {
    nameValue = wrenStringFormat(vm, "$", module);
    wrenPushRoot(vm, AS_OBJ(nameValue));
  }
  
  ObjFiber* fiber = loadModule(vm, nameValue, source);
  if (fiber == NULL)
  {
    wrenPopRoot(vm);
    return WREN_RESULT_COMPILE_ERROR;
  }

  if (module != NULL)
  {
    wrenPopRoot(vm); // nameValue.
  }

  return runInterpreter(vm, fiber);
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
  char* source = vm->config.loadModuleFn(vm, name);
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

Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name)
{
  int symbol = wrenSymbolTableFind(&module->variableNames, name, strlen(name));
  return module->variables.data[symbol];
}

int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length)
{
  if (module->variables.count == MAX_MODULE_VARS) return -2;

  wrenValueBufferWrite(vm, &module->variables, UNDEFINED_VAL);
  return wrenSymbolTableAdd(vm, &module->variableNames, name, length);
}

int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,
                       size_t length, Value value)
{
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

int wrenGetArgumentCount(WrenVM* vm)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  return vm->foreignCallNumArgs;
}

static void validateForeignArgument(WrenVM* vm, int index)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(index >= 0, "index cannot be negative.");
  ASSERT(index < vm->foreignCallNumArgs, "Not that many arguments.");
}

bool wrenGetArgumentBool(WrenVM* vm, int index)
{
  validateForeignArgument(vm, index);

  if (!IS_BOOL(*(vm->foreignCallSlot + index))) return false;

  return AS_BOOL(*(vm->foreignCallSlot + index));
}

double wrenGetArgumentDouble(WrenVM* vm, int index)
{
  validateForeignArgument(vm, index);

  if (!IS_NUM(*(vm->foreignCallSlot + index))) return 0.0;

  return AS_NUM(*(vm->foreignCallSlot + index));
}

void* wrenGetArgumentForeign(WrenVM* vm, int index)
{
  validateForeignArgument(vm, index);
  
  if (!IS_FOREIGN(*(vm->foreignCallSlot + index))) return NULL;
  
  return AS_FOREIGN(*(vm->foreignCallSlot + index))->data;
}

const char* wrenGetArgumentString(WrenVM* vm, int index)
{
  validateForeignArgument(vm, index);

  if (!IS_STRING(*(vm->foreignCallSlot + index))) return NULL;

  return AS_CSTRING(*(vm->foreignCallSlot + index));
}

WrenValue* wrenGetArgumentValue(WrenVM* vm, int index)
{
  validateForeignArgument(vm, index);

  return wrenCaptureValue(vm, *(vm->foreignCallSlot + index));
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

void wrenReturnValue(WrenVM* vm, WrenValue* value)
{
  ASSERT(vm->foreignCallSlot != NULL, "Must be in foreign call.");
  ASSERT(value != NULL, "Value cannot be NULL.");
  
  *vm->foreignCallSlot = value->value;
  vm->foreignCallSlot = NULL;
}
