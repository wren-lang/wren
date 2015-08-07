#include <string.h>

#include "get_value.h"

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

WrenForeignMethodFn getValueBindForeign(const char* signature)
{
  if (strcmp(signature, "static Api.value=(_)") == 0) return setValue;
  if (strcmp(signature, "static Api.value") == 0) return getValue;

  return NULL;
}
