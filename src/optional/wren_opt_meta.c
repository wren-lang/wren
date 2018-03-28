#include "wren_opt_meta.h"

#if WREN_OPT_META

#include <string.h>

#include "wren_vm.h"
#include "wren_opt_meta.wren.inc"

void metaCompile(WrenVM* vm)
{
  const char* source = wrenGetSlotString(vm, 1);
  bool isExpression = wrenGetSlotBool(vm, 2);
  bool printErrors = wrenGetSlotBool(vm, 3);

  // TODO: Allow passing in module?
  ObjClosure* closure = wrenCompileSource(vm, "main", source,
                                          isExpression, printErrors);

  // Return the result. We can't use the public API for this since we have a
  // bare ObjClosure*.
  if (closure == NULL)
  {
    vm->apiStack[0] = NULL_VAL;
  }
  else
  {
    vm->apiStack[0] = OBJ_VAL(closure);
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
    names->elements.data[i] = OBJ_VAL(module->variableNames.data[i]);
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
