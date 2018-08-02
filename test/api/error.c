#include <stdio.h>
#include <string.h>

#include "error.h"

static void runtimeError(WrenFiber* fiber)
{
  wrenSetSlotCount(fiber, 1);
  wrenSetSlotString(fiber, 0, "Error!");
  wrenAbortFiber(fiber, 0);
}

WrenForeignMethodFn errorBindMethod(const char* signature)
{
  if (strcmp(signature, "static Error.runtimeError") == 0) return runtimeError;

  return NULL;
}
