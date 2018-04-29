#include <stdarg.h>
#include <string.h>

#include "wren.h"
#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_core.h"
#include "wren_debug.h"
#include "wren_vm.h"

#if WREN_OPT_META
  #include "wren_opt_meta.h"
#endif
#if WREN_OPT_RANDOM
  #include "wren_opt_random.h"
#endif

#if WREN_DEBUG_TRACE_MEMORY || WREN_DEBUG_TRACE_GC
  #include <time.h>
  #include <stdio.h>
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
  config->errorFn = NULL;
  config->initialHeapSize = 1024 * 1024 * 10;
  config->minHeapSize = 1024 * 1024;
  config->heapGrowthPercent = 50;
  config->userData = NULL;
}

WrenVM* wrenNewVM(WrenConfiguration* config)
{
  WrenReallocateFn reallocate = defaultReallocate;
  if (config != NULL) reallocate = config->reallocateFn;
  
  WrenVM* vm = (WrenVM*)reallocate(NULL, sizeof(*vm));
  memset(vm, 0, sizeof(WrenVM));

  // Copy the configuration if given one.
  if (config != NULL)
  {
    memcpy(&vm->config, config, sizeof(WrenConfiguration));
  }
  else
  {
    wrenInitConfiguration(&vm->config);
  }

  // TODO: Should we allocate and free this during a GC?
  vm->grayCount = 0;
  // TODO: Tune this.
  vm->grayCapacity = 4;
  vm->gray = (Obj**)reallocate(NULL, vm->grayCapacity * sizeof(Obj*));
  vm->nextGC = vm->config.initialHeapSize;

  wrenSymbolTableInit(&vm->methodNames);

  vm->modules = wrenNewMap(vm);
  wrenInitializeCore(vm);
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

  // Free up the GC gray set.
  vm->gray = (Obj**)vm->config.reallocateFn(vm->gray, 0);

  // Tell the user if they didn't free any handles. We don't want to just free
  // them here because the host app may still have pointers to them that they
  // may try to use. Better to tell them about the bug early.
  ASSERT(vm->handles == NULL, "All handles have not been released.");

  wrenSymbolTableClear(vm, &vm->methodNames);

  DEALLOCATE(vm, vm);
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

  wrenGrayObj(vm, (Obj*)vm->modules);

  // Temporary roots.
  for (int i = 0; i < vm->numTempRoots; i++)
  {
    wrenGrayObj(vm, vm->tempRoots[i]);
  }

  // The current fiber.
  wrenGrayObj(vm, (Obj*)vm->fiber);

  // The handles.
  for (WrenHandle* handle = vm->handles;
       handle != NULL;
       handle = handle->next)
  {
    wrenGrayValue(vm, handle->value);
  }

  // Any object the compiler is using (if there is one).
  if (vm->compiler != NULL) wrenMarkCompiler(vm, vm->compiler);

  // Method names.
  wrenBlackenSymbolTable(vm, &vm->methodNames);

  // Now that we have grayed the roots, do a depth-first search over all of the
  // reachable objects.
  wrenBlackenObjects(vm);

  // Collect the white objects.
  Obj** obj = &vm->first;
  while (*obj != NULL)
  {
    if (!((*obj)->isDark))
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
      (*obj)->isDark = false;
      obj = &(*obj)->next;
    }
  }

  // Calculate the next gc point, this is the current allocation plus
  // a configured percentage of the current allocation.
  vm->nextGC = vm->bytesAllocated + ((vm->bytesAllocated * vm->config.heapGrowthPercent) / 100);
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
  WrenForeignMethodFn method = NULL;
  
  if (vm->config.bindForeignMethodFn != NULL)
  {
    method = vm->config.bindForeignMethodFn(vm, moduleName, className, isStatic,
                                            signature);
  }
  
  // If the host didn't provide it, see if it's an optional one.
  if (method == NULL)
  {
#if WREN_OPT_META
    if (strcmp(moduleName, "meta") == 0)
    {
      method = wrenMetaBindForeignMethod(vm, className, isStatic, signature);
    }
#endif
#if WREN_OPT_RANDOM
    if (strcmp(moduleName, "random") == 0)
    {
      method = wrenRandomBindForeignMethod(vm, className, isStatic, signature);
    }
#endif
  }

  return method;
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
    method.fn.obj = AS_CLOSURE(methodValue);
    method.type = METHOD_BLOCK;

    // Patch up the bytecode now that we know the superclass.
    wrenBindMethodCode(classObj, method.fn.obj->fn);
  }

  wrenBindMethod(vm, classObj, symbol, method);
}

static void callForeign(WrenVM* vm, ObjFiber* fiber,
                        WrenForeignMethodFn foreign, int numArgs)
{
  // Save the current state so we can restore it when done.
  Value* apiStack = vm->apiStack;
  vm->apiStack = fiber->stackTop - numArgs;

  foreign(vm);

  // Discard the stack slots for the arguments and temporaries but leave one
  // for the result.
  fiber->stackTop = vm->apiStack + 1;
  vm->apiStack = apiStack;
}

// Handles the current fiber having aborted because of an error.
//
// Walks the call chain of fibers, aborting each one until it hits a fiber that
// handles the error. If none do, tells the VM to stop.
static void runtimeError(WrenVM* vm)
{
  ASSERT(!IS_NULL(vm->fiber->error), "Should only call this after an error.");

  ObjFiber* current = vm->fiber;
  Value error = current->error;
  
  while (current != NULL)
  {
    // Every fiber along the call chain gets aborted with the same error.
    current->error = error;

    // If the caller ran this fiber using "try", give it the error and stop.
    if (current->callerIsTrying)
    {
      // Make the caller's try method return the error message.
      current->caller->stackTop[-1] = vm->fiber->error;
      vm->fiber = current->caller;
      return;
    }
    
    // Otherwise, unhook the caller since we will never resume and return to it.
    ObjFiber* caller = current->caller;
    current->caller = NULL;
    current = caller;
  }

  // If we got here, nothing caught the error, so show the stack trace.
  wrenDebugPrintStackTrace(vm);
  vm->fiber = NULL;
  vm->apiStack = NULL;
}

// Aborts the current fiber with an appropriate method not found error for a
// method with [symbol] on [classObj].
static void methodNotFound(WrenVM* vm, ObjClass* classObj, int symbol)
{
  vm->fiber->error = wrenStringFormat(vm, "@ does not implement '$'.",
      OBJ_VAL(classObj->name), vm->methodNames.data[symbol]->value);
}

// Checks that [value], which must be a closure, does not require more
// parameters than are provided by [numArgs].
//
// If there are not enough arguments, aborts the current fiber and returns
// `false`.
static bool checkArity(WrenVM* vm, Value value, int numArgs)
{
  ASSERT(IS_CLOSURE(value), "Receiver must be a closure.");
  ObjFn* fn = AS_CLOSURE(value)->fn;

  // We only care about missing arguments, not extras. The "- 1" is because
  // numArgs includes the receiver, the function itself, which we don't want to
  // count.
  if (numArgs - 1 >= fn->arity) return true;

  vm->fiber->error = CONST_STRING(vm, "Function expects more arguments.");
  return false;
}

// Pushes [closure] onto [fiber]'s callstack and invokes it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
static inline void callFunction(
    WrenVM* vm, ObjFiber* fiber, ObjClosure* closure, int numArgs)
{
  // Grow the call frame array if needed.
  if (fiber->numFrames + 1 > fiber->frameCapacity)
  {
    int max = fiber->frameCapacity * 2;
    fiber->frames = (CallFrame*)wrenReallocate(vm, fiber->frames,
        sizeof(CallFrame) * fiber->frameCapacity,
        sizeof(CallFrame) * max);
    fiber->frameCapacity = max;
  }
  
  // Grow the stack if needed.
  int stackSize = (int)(fiber->stackTop - fiber->stack);
  int needed = stackSize + closure->fn->maxSlots;
  wrenEnsureStack(vm, fiber, needed);
  
  wrenAppendCallFrame(vm, fiber, closure, fiber->stackTop - numArgs);
}

// Looks up the previously loaded module with [name].
//
// Returns `NULL` if no module with that name has been loaded.
static ObjModule* getModule(WrenVM* vm, Value name)
{
  Value moduleValue = wrenMapGet(vm->modules, name);
  return !IS_UNDEFINED(moduleValue) ? AS_MODULE(moduleValue) : NULL;
}

static ObjClosure* compileInModule(WrenVM* vm, Value name, const char* source,
                                   bool isExpression, bool printErrors)
{
  // See if the module has already been loaded.
  ObjModule* module = getModule(vm, name);
  if (module == NULL)
  {
    module = wrenNewModule(vm, AS_STRING(name));

    // Store it in the VM's module registry so we don't load the same module
    // multiple times.
    wrenMapSet(vm, vm->modules, name, OBJ_VAL(module));

    // Implicitly import the core module.
    ObjModule* coreModule = getModule(vm, NULL_VAL);
    for (int i = 0; i < coreModule->variables.count; i++)
    {
      wrenDefineVariable(vm, module,
                         coreModule->variableNames.data[i]->value,
                         coreModule->variableNames.data[i]->length,
                         coreModule->variables.data[i]);
    }
  }

  ObjFn* fn = wrenCompile(vm, module, source, isExpression, printErrors);
  if (fn == NULL)
  {
    // TODO: Should we still store the module even if it didn't compile?
    return NULL;
  }

  // Functions are always wrapped in closures.
  wrenPushRoot(vm, (Obj*)fn);
  ObjClosure* closure = wrenNewClosure(vm, fn);
  wrenPopRoot(vm); // fn.

  return closure;
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
  WrenForeignClassMethods methods;
  methods.allocate = NULL;
  methods.finalize = NULL;
  
  // Check the optional built-in module first so the host can override it.
  
  if (vm->config.bindForeignClassFn != NULL)
  {
    methods = vm->config.bindForeignClassFn(vm, module->name->value,
                                            classObj->name->value);
  }

  // If the host didn't provide it, see if it's a built in optional module.
  if (methods.allocate == NULL && methods.finalize == NULL)
  {
#if WREN_OPT_RANDOM
    if (strcmp(module->name->value, "random") == 0)
    {
      methods = wrenRandomBindForeignClass(vm, module->name->value,
                                           classObj->name->value);
    }
#endif
  }
  
  Method method;
  method.type = METHOD_FOREIGN;

  // Add the symbol even if there is no allocator so we can ensure that the
  // symbol itself is always in the symbol table.
  int symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, "<allocate>", 10);
  if (methods.allocate != NULL)
  {
    method.fn.foreign = methods.allocate;
    wrenBindMethod(vm, classObj, symbol, method);
  }
  
  // Add the symbol even if there is no finalizer so we can ensure that the
  // symbol itself is always in the symbol table.
  symbol = wrenSymbolTableEnsure(vm, &vm->methodNames, "<finalize>", 10);
  if (methods.finalize != NULL)
  {
    method.fn.foreign = (WrenForeignMethodFn)methods.finalize;
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
  Value* oldApiStack = vm->apiStack;
  vm->apiStack = stack;

  method->fn.foreign(vm);

  vm->apiStack = oldApiStack;
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

  WrenFinalizerFn finalizer = (WrenFinalizerFn)method->fn.foreign;
  finalizer(foreign->data);
}

Value wrenImportModule(WrenVM* vm, Value name)
{
  // If the module is already loaded, we don't need to do anything.
  if (!IS_UNDEFINED(wrenMapGet(vm->modules, name))) return NULL_VAL;
  
  const char* source = NULL;
  bool allocatedSource = true;
  
  // Let the host try to provide the module.
  if (vm->config.loadModuleFn != NULL)
  {
    source = vm->config.loadModuleFn(vm, AS_CSTRING(name));
  }
  
  // If the host didn't provide it, see if it's a built in optional module.
  if (source == NULL)
  {
    ObjString* nameString = AS_STRING(name);
#if WREN_OPT_META
    if (strcmp(nameString->value, "meta") == 0) source = wrenMetaSource();
#endif
#if WREN_OPT_RANDOM
    if (strcmp(nameString->value, "random") == 0) source = wrenRandomSource();
#endif
    
    // TODO: Should we give the host the ability to provide strings that don't
    // need to be freed?
    allocatedSource = false;
  }
  
  if (source == NULL)
  {
    vm->fiber->error = wrenStringFormat(vm, "Could not load module '@'.", name);
    return NULL_VAL;
  }
  
  ObjClosure* moduleClosure = compileInModule(vm, name, source, false, true);
  
  // Modules loaded by the host are expected to be dynamically allocated with
  // ownership given to the VM, which will free it. The built in optional
  // modules are constant strings which don't need to be freed.
  if (allocatedSource) DEALLOCATE(vm, (char*)source);
  
  if (moduleClosure == NULL)
  {
    vm->fiber->error = wrenStringFormat(vm,
                                        "Could not compile module '@'.", name);
    return NULL_VAL;
  }
  
  // Return the closure that executes the module.
  return OBJ_VAL(moduleClosure);
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
      fn = frame->closure->fn;

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
          wrenDumpInstruction(vm, fn, (int)(ip - fn->code.data)); \
        }                                                         \
        while (false)
  #else
    #define DEBUG_TRACE_INSTRUCTIONS() do { } while (false)
  #endif

  #if WREN_COMPUTED_GOTO

  static void* dispatchTable[] = {
    #define OPCODE(name, _) &&code_##name,
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
    CASE_CODE(NULL):  PUSH(NULL_VAL); DISPATCH();
    CASE_CODE(FALSE): PUSH(FALSE_VAL); DISPATCH();
    CASE_CODE(TRUE):  PUSH(TRUE_VAL); DISPATCH();

    CASE_CODE(STORE_LOCAL):
      stackStart[READ_BYTE()] = PEEK();
      DISPATCH();

    CASE_CODE(CONSTANT):
      PUSH(fn->constants.data[READ_SHORT()]);
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
      classObj = AS_CLASS(fn->constants.data[READ_SHORT()]);
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
          if (!IS_NULL(fiber->error)) RUNTIME_ERROR();
          break;

        case METHOD_FN_CALL:
          if (!checkArity(vm, args[0], numArgs)) RUNTIME_ERROR();

          STORE_FRAME();
          callFunction(vm, fiber, AS_CLOSURE(args[0]), numArgs);
          LOAD_FRAME();
          break;

        case METHOD_BLOCK:
          STORE_FRAME();
          callFunction(vm, fiber, (ObjClosure*)method->fn.obj, numArgs);
          LOAD_FRAME();
          break;

        case METHOD_NONE:
          UNREACHABLE();
          break;
      }
      DISPATCH();
    }

    CASE_CODE(LOAD_UPVALUE):
    {
      ObjUpvalue** upvalues = frame->closure->upvalues;
      PUSH(*upvalues[READ_BYTE()]->value);
      DISPATCH();
    }

    CASE_CODE(STORE_UPVALUE):
    {
      ObjUpvalue** upvalues = frame->closure->upvalues;
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
        // See if there's another fiber to return to. If not, we're done.
        if (fiber->caller == NULL)
        {
          // Store the final result value at the beginning of the stack so the
          // C API can get it.
          fiber->stack[0] = result;
          fiber->stackTop = fiber->stack + 1;
          return WREN_RESULT_SUCCESS;
        }
        
        ObjFiber* resumingFiber = fiber->caller;
        fiber->caller = NULL;
        fiber = resumingFiber;
        vm->fiber = resumingFiber;
        
        // Store the result in the resuming fiber.
        fiber->stackTop[-1] = result;
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
      // Create the closure and push it on the stack before creating upvalues
      // so that it doesn't get collected.
      ObjFn* function = AS_FN(fn->constants.data[READ_SHORT()]);
      ObjClosure* closure = wrenNewClosure(vm, function);
      PUSH(OBJ_VAL(closure));

      // Capture upvalues, if any.
      for (int i = 0; i < function->numUpvalues; i++)
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
          closure->upvalues[i] = frame->closure->upvalues[index];
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
    
    CASE_CODE(IMPORT_MODULE):
    {
      Value name = fn->constants.data[READ_SHORT()];

      // Make a slot on the stack for the module's fiber to place the return
      // value. It will be popped after this fiber is resumed. Store the
      // imported module's closure in the slot in case a GC happens when
      // invoking the closure.
      PUSH(wrenImportModule(vm, name));
      
      if (!IS_NULL(fiber->error)) RUNTIME_ERROR();

      // If we get a closure, call it to execute the module body.
      if (IS_CLOSURE(PEEK()))
      {
        STORE_FRAME();
        ObjClosure* closure = AS_CLOSURE(PEEK());
        callFunction(vm, fiber, closure, 1);
        LOAD_FRAME();
      }

      DISPATCH();
    }
    
    CASE_CODE(IMPORT_VARIABLE):
    {
      Value module = fn->constants.data[READ_SHORT()];
      Value variable = fn->constants.data[READ_SHORT()];
      
      Value result = wrenGetModuleVariable(vm, module, variable);
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
  return WREN_RESULT_RUNTIME_ERROR;

  #undef READ_BYTE
  #undef READ_SHORT
}

WrenHandle* wrenMakeCallHandle(WrenVM* vm, const char* signature)
{
  ASSERT(signature != NULL, "Signature cannot be NULL.");
  
  int signatureLength = (int)strlen(signature);
  ASSERT(signatureLength > 0, "Signature cannot be empty.");
  
  // Count the number parameters the method expects.
  int numParams = 0;
  if (signature[signatureLength - 1] == ')')
  {
    for (int i = signatureLength - 1; i > 0 && signature[i] != '('; i--)
    {
      if (signature[i] == '_') numParams++;
    }
  }
  
  // Count subscript arguments.
  if (signature[0] == '[')
  {
    for (int i = 0; i < signatureLength && signature[i] != ']'; i++)
    {
      if (signature[i] == '_') numParams++;
    }
  }
  
  // Add the signatue to the method table.
  int method =  wrenSymbolTableEnsure(vm, &vm->methodNames,
                                      signature, signatureLength);
  
  // Create a little stub function that assumes the arguments are on the stack
  // and calls the method.
  ObjFn* fn = wrenNewFunction(vm, NULL, numParams + 1);
  
  // Wrap the function in a closure and then in a handle. Do this here so it
  // doesn't get collected as we fill it in.
  WrenHandle* value = wrenMakeHandle(vm, OBJ_VAL(fn));
  value->value = OBJ_VAL(wrenNewClosure(vm, fn));
  
  wrenByteBufferWrite(vm, &fn->code, (uint8_t)(CODE_CALL_0 + numParams));
  wrenByteBufferWrite(vm, &fn->code, (method >> 8) & 0xff);
  wrenByteBufferWrite(vm, &fn->code, method & 0xff);
  wrenByteBufferWrite(vm, &fn->code, CODE_RETURN);
  wrenByteBufferWrite(vm, &fn->code, CODE_END);
  wrenIntBufferFill(vm, &fn->debug->sourceLines, 0, 5);
  wrenFunctionBindName(vm, fn, signature, signatureLength);

  return value;
}

WrenInterpretResult wrenCall(WrenVM* vm, WrenHandle* method)
{
  ASSERT(method != NULL, "Method cannot be NULL.");
  ASSERT(IS_CLOSURE(method->value), "Method must be a method handle.");
  ASSERT(vm->fiber != NULL, "Must set up arguments for call first.");
  ASSERT(vm->apiStack != NULL, "Must set up arguments for call first.");
  ASSERT(vm->fiber->numFrames == 0, "Can not call from a foreign method.");
  
  ObjClosure* closure = AS_CLOSURE(method->value);
  
  ASSERT(vm->fiber->stackTop - vm->fiber->stack >= closure->fn->arity,
         "Stack must have enough arguments for method.");
  
  // Discard any extra temporary slots. We take for granted that the stub
  // function has exactly one slot for each argument.
  vm->fiber->stackTop = &vm->fiber->stack[closure->fn->maxSlots];
  
  callFunction(vm, vm->fiber, closure, 0);
  return runInterpreter(vm, vm->fiber);
}

WrenHandle* wrenMakeHandle(WrenVM* vm, Value value)
{
  if (IS_OBJ(value)) wrenPushRoot(vm, AS_OBJ(value));
  
  // Make a handle for it.
  WrenHandle* handle = ALLOCATE(vm, WrenHandle);
  handle->value = value;

  if (IS_OBJ(value)) wrenPopRoot(vm);

  // Add it to the front of the linked list of handles.
  if (vm->handles != NULL) vm->handles->prev = handle;
  handle->prev = NULL;
  handle->next = vm->handles;
  vm->handles = handle;
  
  return handle;
}

void wrenReleaseHandle(WrenVM* vm, WrenHandle* handle)
{
  ASSERT(handle != NULL, "Handle cannot be NULL.");

  // Update the VM's head pointer if we're releasing the first handle.
  if (vm->handles == handle) vm->handles = handle->next;

  // Unlink it from the list.
  if (handle->prev != NULL) handle->prev->next = handle->next;
  if (handle->next != NULL) handle->next->prev = handle->prev;

  // Clear it out. This isn't strictly necessary since we're going to free it,
  // but it makes for easier debugging.
  handle->prev = NULL;
  handle->next = NULL;
  handle->value = NULL_VAL;
  DEALLOCATE(vm, handle);
}

WrenInterpretResult wrenInterpret(WrenVM* vm, const char* source)
{
  return wrenInterpretInModule(vm, "main", source);
}

WrenInterpretResult wrenInterpretInModule(WrenVM* vm, const char* module,
                                          const char* source)
{
  ObjClosure* closure = wrenCompileSource(vm, module, source, false, true);
  if (closure == NULL) return WREN_RESULT_COMPILE_ERROR;
  
  wrenPushRoot(vm, (Obj*)closure);
  ObjFiber* fiber = wrenNewFiber(vm, closure);
  wrenPopRoot(vm); // closure.
  
  return runInterpreter(vm, fiber);
}

ObjClosure* wrenCompileSource(WrenVM* vm, const char* module, const char* source,
                            bool isExpression, bool printErrors)
{
  Value nameValue = NULL_VAL;
  if (module != NULL)
  {
    nameValue = wrenNewString(vm, module);
    wrenPushRoot(vm, AS_OBJ(nameValue));
  }
  
  ObjClosure* closure = compileInModule(vm, nameValue, source,
                                        isExpression, printErrors);

  if (module != NULL) wrenPopRoot(vm); // nameValue.
  return closure;
}

Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName)
{
  ObjModule* module = getModule(vm, moduleName);
  if (module == NULL)
  {
    vm->fiber->error = wrenStringFormat(vm, "Module '@' is not loaded.",
                                        moduleName);
    return NULL_VAL;
  }
  
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

Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name)
{
  int symbol = wrenSymbolTableFind(&module->variableNames, name, strlen(name));
  return module->variables.data[symbol];
}

int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length, int line)
{
  if (module->variables.count == MAX_MODULE_VARS) return -2;

  // Implicitly defined variables get a "value" that is the line where the
  // variable is first used. We'll use that later to report an error on the
  // right line.
  wrenValueBufferWrite(vm, &module->variables, NUM_VAL(line));
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
  else if (IS_NUM(module->variables.data[symbol]))
  {
    // An implicitly declared variable's value will always be a number. Now we
    // have a real definition.
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

int wrenGetSlotCount(WrenVM* vm)
{
  if (vm->apiStack == NULL) return 0;
  
  return (int)(vm->fiber->stackTop - vm->apiStack);
}

void wrenEnsureSlots(WrenVM* vm, int numSlots)
{
  // If we don't have a fiber accessible, create one for the API to use.
  if (vm->apiStack == NULL)
  {
    vm->fiber = wrenNewFiber(vm, NULL);
    vm->apiStack = vm->fiber->stack;
  }
  
  int currentSize = (int)(vm->fiber->stackTop - vm->apiStack);
  if (currentSize >= numSlots) return;
  
  // Grow the stack if needed.
  int needed = (int)(vm->apiStack - vm->fiber->stack) + numSlots;
  wrenEnsureStack(vm, vm->fiber, needed);
  
  vm->fiber->stackTop = vm->apiStack + numSlots;
}

// Ensures that [slot] is a valid index into the API's stack of slots.
static void validateApiSlot(WrenVM* vm, int slot)
{
  ASSERT(slot >= 0, "Slot cannot be negative.");
  ASSERT(slot < wrenGetSlotCount(vm), "Not that many slots.");
}

// Gets the type of the object in [slot].
WrenType wrenGetSlotType(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  if (IS_BOOL(vm->apiStack[slot])) return WREN_TYPE_BOOL;
  if (IS_NUM(vm->apiStack[slot])) return WREN_TYPE_NUM;
  if (IS_FOREIGN(vm->apiStack[slot])) return WREN_TYPE_FOREIGN;
  if (IS_LIST(vm->apiStack[slot])) return WREN_TYPE_LIST;
  if (IS_NULL(vm->apiStack[slot])) return WREN_TYPE_NULL;
  if (IS_STRING(vm->apiStack[slot])) return WREN_TYPE_STRING;
  
  return WREN_TYPE_UNKNOWN;
}

bool wrenGetSlotBool(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_BOOL(vm->apiStack[slot]), "Slot must hold a bool.");

  return AS_BOOL(vm->apiStack[slot]);
}

const char* wrenGetSlotBytes(WrenVM* vm, int slot, int* length)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_STRING(vm->apiStack[slot]), "Slot must hold a string.");
  
  ObjString* string = AS_STRING(vm->apiStack[slot]);
  *length = string->length;
  return string->value;
}

double wrenGetSlotDouble(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_NUM(vm->apiStack[slot]), "Slot must hold a number.");

  return AS_NUM(vm->apiStack[slot]);
}

void* wrenGetSlotForeign(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_FOREIGN(vm->apiStack[slot]),
         "Slot must hold a foreign instance.");

  return AS_FOREIGN(vm->apiStack[slot])->data;
}

const char* wrenGetSlotString(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_STRING(vm->apiStack[slot]), "Slot must hold a string.");

  return AS_CSTRING(vm->apiStack[slot]);
}

WrenHandle* wrenGetSlotHandle(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  return wrenMakeHandle(vm, vm->apiStack[slot]);
}

// Stores [value] in [slot] in the foreign call stack.
static void setSlot(WrenVM* vm, int slot, Value value)
{
  validateApiSlot(vm, slot);
  vm->apiStack[slot] = value;
}

void wrenSetSlotBool(WrenVM* vm, int slot, bool value)
{
  setSlot(vm, slot, BOOL_VAL(value));
}

void wrenSetSlotBytes(WrenVM* vm, int slot, const char* bytes, size_t length)
{
  ASSERT(bytes != NULL, "Byte array cannot be NULL.");
  setSlot(vm, slot, wrenNewStringLength(vm, bytes, length));
}

void wrenSetSlotDouble(WrenVM* vm, int slot, double value)
{
  setSlot(vm, slot, NUM_VAL(value));
}

void* wrenSetSlotNewForeign(WrenVM* vm, int slot, int classSlot, size_t size)
{
  validateApiSlot(vm, slot);
  validateApiSlot(vm, classSlot);
  ASSERT(IS_CLASS(vm->apiStack[classSlot]), "Slot must hold a class.");
  
  ObjClass* classObj = AS_CLASS(vm->apiStack[classSlot]);
  ASSERT(classObj->numFields == -1, "Class must be a foreign class.");
  
  ObjForeign* foreign = wrenNewForeign(vm, classObj, size);
  vm->apiStack[slot] = OBJ_VAL(foreign);
  
  return (void*)foreign->data;
}

void wrenSetSlotNewList(WrenVM* vm, int slot)
{
  setSlot(vm, slot, OBJ_VAL(wrenNewList(vm, 0)));
}

void wrenSetSlotNull(WrenVM* vm, int slot)
{
  setSlot(vm, slot, NULL_VAL);
}

void wrenSetSlotString(WrenVM* vm, int slot, const char* text)
{
  ASSERT(text != NULL, "String cannot be NULL.");
  
  setSlot(vm, slot, wrenNewString(vm, text));
}

void wrenSetSlotHandle(WrenVM* vm, int slot, WrenHandle* handle)
{
  ASSERT(handle != NULL, "Handle cannot be NULL.");

  setSlot(vm, slot, handle->value);
}

int wrenGetListCount(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  ASSERT(IS_LIST(vm->apiStack[slot]), "Slot must hold a list.");
  
  ValueBuffer elements = AS_LIST(vm->apiStack[slot])->elements;
  return elements.count;
}

void wrenGetListElement(WrenVM* vm, int listSlot, int index, int elementSlot)
{
  validateApiSlot(vm, listSlot);
  validateApiSlot(vm, elementSlot);
  ASSERT(IS_LIST(vm->apiStack[listSlot]), "Slot must hold a list.");
  
  ValueBuffer elements = AS_LIST(vm->apiStack[listSlot])->elements;
  vm->apiStack[elementSlot] = elements.data[index];
}

void wrenInsertInList(WrenVM* vm, int listSlot, int index, int elementSlot)
{
  validateApiSlot(vm, listSlot);
  validateApiSlot(vm, elementSlot);
  ASSERT(IS_LIST(vm->apiStack[listSlot]), "Must insert into a list.");
  
  ObjList* list = AS_LIST(vm->apiStack[listSlot]);
  
  // Negative indices count from the end.
  if (index < 0) index = list->elements.count + 1 + index;
  
  ASSERT(index <= list->elements.count, "Index out of bounds.");
  
  wrenListInsert(vm, list, vm->apiStack[elementSlot], index);
}

void wrenGetVariable(WrenVM* vm, const char* module, const char* name,
                     int slot)
{
  ASSERT(module != NULL, "Module cannot be NULL.");
  ASSERT(name != NULL, "Variable name cannot be NULL.");  
  validateApiSlot(vm, slot);
  
  Value moduleName = wrenStringFormat(vm, "$", module);
  wrenPushRoot(vm, AS_OBJ(moduleName));
  
  ObjModule* moduleObj = getModule(vm, moduleName);
  ASSERT(moduleObj != NULL, "Could not find module.");
  
  wrenPopRoot(vm); // moduleName.

  int variableSlot = wrenSymbolTableFind(&moduleObj->variableNames,
                                         name, strlen(name));
  ASSERT(variableSlot != -1, "Could not find variable.");
  
  setSlot(vm, slot, moduleObj->variables.data[variableSlot]);
}

void wrenAbortFiber(WrenVM* vm, int slot)
{
  validateApiSlot(vm, slot);
  vm->fiber->error = vm->apiStack[slot];
}

void* wrenGetUserData(WrenVM* vm)
{
	return vm->config.userData;
}

void wrenSetUserData(WrenVM* vm, void* userData)
{
	vm->config.userData = userData;
}
