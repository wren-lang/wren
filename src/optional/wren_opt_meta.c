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
  ASSERT(strcmp(signature, "compile_(_,_,_)") == 0, "Should be compile method.");
  
  return metaCompile;
}

#endif
