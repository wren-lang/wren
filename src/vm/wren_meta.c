#include "wren_meta.h"

#if WREN_USE_LIB_META

#include <string.h>

// This string literal is generated automatically from meta.wren. Do not edit.
static const char* libSource =
"class Meta {\n"
"  foreign static eval(source)\n"
"}\n";

void metaEval(WrenVM* vm)
{
  const char* source = wrenGetArgumentString(vm, 1);
  // TODO: Type check argument.
  wrenInterpret(vm, "Meta", source);
}

void wrenLoadMetaLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", libSource);
}

WrenForeignMethodFn wrenBindMetaForeignMethod(WrenVM* vm,
                                              const char* className,
                                              const char* signature)
{
  if (strcmp(className, "Meta") != 0) return NULL;

  if (strcmp(signature, "eval(_)") == 0) return metaEval;
  return NULL;
}

#endif
