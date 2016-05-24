#include "wren_opt_meta.h"

#if WREN_OPT_META

#include <string.h>

#include "wren_vm.h"
#include "wren_opt_meta.wren.inc"

void metaCompile(WrenVM* vm)
{
  // Evaluate the code in the module where the calling function was defined.
  // That's one stack frame back from the top since the top-most frame is the
  // helper eval() method in Meta itself.
  ObjClosure* caller = vm->fiber->frames[vm->fiber->numFrames - 2].closure;
  ObjModule* module = caller->fn->module;

  const char* source = wrenGetSlotString(vm, 1);
  bool isExpression = wrenGetSlotBool(vm, 2);
  bool printErrors = wrenGetSlotBool(vm, 3);
  
  // Compile it.
  ObjFn* fn = wrenCompile(vm, module, source, isExpression, printErrors);

  // Return the result. We can't use the public API for this since we have a
  // bare ObjFn.
  if (fn == NULL)
  {
    vm->apiStack[0] = NULL_VAL;
  }
  else
  {
    wrenPushRoot(vm, (Obj*)fn);
    vm->apiStack[0] = OBJ_VAL(wrenNewClosure(vm, fn));
    wrenPopRoot(vm);
  }
}

void metaGetModuleVariables(WrenVM* vm) {
  wrenEnsureSlots(vm, 3);
  
  Value moduleValue = wrenMapGet(vm->modules, vm->apiStack[1]);
  if (IS_UNDEFINED(moduleValue))
  {
    vm->apiStack[0] = NULL_VAL;
    return;
  }
    
  ObjModule* module = AS_MODULE(moduleValue);
  ObjList* names = wrenNewList(vm, module->variableNames.count);
  vm->apiStack[0] = OBJ_VAL(names);

  // Initialize the elements to null in case a collection happens when we
  // allocate the strings below.
  for (int i = 0; i < names->elements.count; i++)
  {
    names->elements.data[i] = NULL_VAL;
  }
  
  for (int i = 0; i < names->elements.count; i++)
  {
    String* name = &module->variableNames.data[i];
    names->elements.data[i] = wrenNewString(vm, name->buffer, name->length);
  }
}

const char* wrenMetaSource()
{
  return metaModuleSource;
}

WrenForeignMethodFn wrenMetaBindForeignMethod(WrenVM* vm,
                                              const char* className,
                                              bool isStatic,
                                              const char* signature)
{
  // There is only one foreign method in the meta module.
  ASSERT(strcmp(className, "Meta") == 0, "Should be in Meta class.");
  ASSERT(isStatic, "Should be static.");
  
  if (strcmp(signature, "compile_(_,_,_)") == 0)
  {
    return metaCompile;
  }
  
  if (strcmp(signature, "getModuleVariables_(_)") == 0)
  {
    return metaGetModuleVariables;
  }
  
  ASSERT(false, "Unknown method.");
  return NULL;
}

#endif
