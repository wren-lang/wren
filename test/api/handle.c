#include <string.h>

#include "handle.h"

static WrenHandle* handle;

static void setValue(WrenFiber* fiber)
{
  handle = wrenGetSlotHandle(fiber, 1);
}

static void getValue(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenSetSlotHandle(fiber, 0, handle);
  wrenReleaseHandle(vm, handle);
}

WrenForeignMethodFn handleBindMethod(const char* signature)
{
  if (strcmp(signature, "static Handle.value=(_)") == 0) return setValue;
  if (strcmp(signature, "static Handle.value") == 0) return getValue;

  return NULL;
}
