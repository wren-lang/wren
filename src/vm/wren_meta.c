#include "wren_meta.h"

#if WREN_USE_LIB_META

// This string literal is generated automatically from meta.wren. Do not edit.
static const char* libSource =
"class Meta {}\n";

void metaEval(WrenVM* vm)
{
    const char* source = wrenGetArgumentString(vm, 1);
    wrenInterpret(vm, "Meta", source);
}

void wrenLoadMetaLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", libSource);
  wrenDefineStaticMethod(vm, "Meta", "eval(_)", metaEval);
}

#endif
