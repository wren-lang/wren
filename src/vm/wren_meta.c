#include "wren_meta.h"

#if WREN_USE_LIB_META

#include <string.h>

#include "wren_primitive.h"

#include "wren_meta.wren.inc"

DEF_PRIMITIVE(meta_eval)
{
  if (!validateString(vm, args[1], "Source code")) return PRIM_FIBER;

  // Eval the code in the module where the calling function was defined.
  Value callingFn = OBJ_VAL(vm->fiber->frames[vm->fiber->numFrames - 1].fn);
  ObjModule* module = IS_FN(callingFn)
      ? AS_FN(callingFn)->module
      : AS_CLOSURE(callingFn)->fn->module;

  // Compile it.
  ObjFn* fn = wrenCompile(vm, module, "<eval>", AS_CSTRING(args[1]), false);
  if (fn == NULL) RETURN_ERROR("Could not compile source code.");

  // TODO: Include the compile errors in the runtime error message.

  wrenPushRoot(vm, (Obj*)fn);

  // Create a fiber to run the code in.
  ObjFiber* evalFiber = wrenNewFiber(vm, (Obj*)fn);

  // Remember what fiber to return to.
  evalFiber->caller = vm->fiber;

  // Switch to the fiber.
  vm->fiber = evalFiber;
  
  wrenPopRoot(vm);
  return PRIM_FIBER;
}

void wrenLoadMetaLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", metaModuleSource);

  ObjModule* coreModule = wrenGetCoreModule(vm);

  // The methods on "Meta" are static, so get the metaclass for the Meta class.
  ObjClass* meta = AS_CLASS(wrenFindVariable(vm, coreModule, "Meta"));
  PRIMITIVE(meta->obj.classObj, "eval(_)", meta_eval);
}

#endif
