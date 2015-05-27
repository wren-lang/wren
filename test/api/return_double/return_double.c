#include <string.h>

#include "return_double.h"

static void returnInt(WrenVM* vm)
{
  wrenReturnDouble(vm, 123456);
}

static void returnFloat(WrenVM* vm)
{
  wrenReturnDouble(vm, 123.456);
}

WrenForeignMethodFn return_doubleBindForeign(const char* signature)
{
  if (strcmp(signature, "static Api.returnInt") == 0) return returnInt;
  if (strcmp(signature, "static Api.returnFloat") == 0) return returnFloat;

  return NULL;
}
