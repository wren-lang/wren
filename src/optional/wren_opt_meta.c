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
  Value callingFn = OBJ_VAL(vm->fiber->frames[vm->fiber->numFrames - 2].fn);
  ObjModule* module = IS_FN(callingFn)
      ? AS_FN(callingFn)->module
      : AS_CLOSURE(callingFn)->fn->module;

  // Compile it.
  ObjFn* fn = wrenCompile(vm, module, wrenGetArgumentString(vm, 1), false);
  if (fn == NULL) return;

  // Return the result. We can't use the public API for this since we have a
  // bare ObjFn.
  *vm->foreignCallSlot = OBJ_VAL(fn);
  vm->foreignCallSlot = NULL;
}

static WrenForeignMethodFn bindMetaForeignMethods(WrenVM* vm,
                                                  const char* module,
                                                  const char* className,
                                                  bool isStatic,
                                                  const char* signature)
{
  // There is only one foreign method in the meta module.
  ASSERT(strcmp(module, "meta") == 0, "Should be in meta module.");
  ASSERT(strcmp(className, "Meta") == 0, "Should be in Meta class.");
  ASSERT(isStatic, "Should be static.");
  ASSERT(strcmp(signature, "compile_(_)") == 0, "Should be compile method.");

  return metaCompile;
}

void wrenLoadMetaModule(WrenVM* vm)
{
  WrenBindForeignMethodFn previousBindFn = vm->config.bindForeignMethodFn;
  vm->config.bindForeignMethodFn = bindMetaForeignMethods;

  wrenInterpretInModule(vm, "meta", metaModuleSource);

  vm->config.bindForeignMethodFn = previousBindFn;
}

#endif
