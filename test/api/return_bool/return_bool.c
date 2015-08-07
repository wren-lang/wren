#include <string.h>

#include "return_bool.h"

static void returnTrue(WrenVM* vm)
{
  wrenReturnBool(vm, true);
}

static void returnFalse(WrenVM* vm)
{
  wrenReturnBool(vm, false);
}

WrenForeignMethodFn returnBoolBindForeign(const char* signature)
{
  if (strcmp(signature, "static Api.returnTrue") == 0) return returnTrue;
  if (strcmp(signature, "static Api.returnFalse") == 0) return returnFalse;

  return NULL;
}
