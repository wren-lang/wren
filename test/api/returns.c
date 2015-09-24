#include <string.h>

#include "returns.h"

static void implicitNull(WrenVM* vm)
{
  // Do nothing.
}

static void returnInt(WrenVM* vm)
{
  wrenReturnDouble(vm, 123456);
}

static void returnFloat(WrenVM* vm)
{
  wrenReturnDouble(vm, 123.456);
}

static void returnTrue(WrenVM* vm)
{
  wrenReturnBool(vm, true);
}

static void returnFalse(WrenVM* vm)
{
  wrenReturnBool(vm, false);
}

static void returnString(WrenVM* vm)
{
  wrenReturnString(vm, "a string", -1);
}

static void returnBytes(WrenVM* vm)
{
  wrenReturnString(vm, "a\0b\0c", 5);
}

WrenForeignMethodFn returnsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Api.implicitNull") == 0) return implicitNull;
  if (strcmp(signature, "static Api.returnInt") == 0) return returnInt;
  if (strcmp(signature, "static Api.returnFloat") == 0) return returnFloat;
  if (strcmp(signature, "static Api.returnTrue") == 0) return returnTrue;
  if (strcmp(signature, "static Api.returnFalse") == 0) return returnFalse;
  if (strcmp(signature, "static Api.returnString") == 0) return returnString;
  if (strcmp(signature, "static Api.returnBytes") == 0) return returnBytes;

  return NULL;
}
