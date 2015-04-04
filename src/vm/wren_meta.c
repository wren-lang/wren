#include "wren_meta.h"

#if WREN_USE_LIB_META

#include <string.h>

#include "wren_primitive.h"

// This string literal is generated automatically from meta.wren. Do not edit.
static const char* libSource =
"class Meta {}\n";

DEF_PRIMITIVE(meta_eval)
{
  if (!validateString(vm, args, 1, "Source code")) return PRIM_ERROR;

  // Eval the code in the module where the calling function was defined.
  Value callingFn = OBJ_VAL(fiber->frames[fiber->numFrames - 1].fn);
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
  evalFiber->caller = fiber;

  // Switch to the fiber.
  args[0] = OBJ_VAL(evalFiber);

  wrenPopRoot(vm);
  return PRIM_RUN_FIBER;
}

void wrenLoadMetaLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", libSource);

  // The methods on "Meta" are static, so get the metaclass for the Meta class.
  ObjClass* meta = AS_CLASS(wrenFindVariable(vm, "Meta"))->obj.classObj;
  PRIMITIVE(meta, "eval(_)", meta_eval);
}

#endif
