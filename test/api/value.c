#include <string.h>

#include "value.h"

static WrenValue* value;

static void setValue(WrenVM* vm)
{
  value = wrenGetArgumentValue(vm, 1);
}

static void getValue(WrenVM* vm)
{
  wrenReturnValue(vm, value);
  wrenReleaseValue(vm, value);
}

WrenForeignMethodFn valueBindMethod(const char* signature)
{
  if (strcmp(signature, "static Value.value=(_)") == 0) return setValue;
  if (strcmp(signature, "static Value.value") == 0) return getValue;

  return NULL;
}
