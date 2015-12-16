#include <string.h>

#include "benchmark.h"

static void arguments(WrenVM* vm)
{
  double result = 0;

  result += wrenGetSlotDouble(vm, 1);
  result += wrenGetSlotDouble(vm, 2);
  result += wrenGetSlotDouble(vm, 3);
  result += wrenGetSlotDouble(vm, 4);

  wrenSetSlotDouble(vm, 0, result);
}

WrenForeignMethodFn benchmarkBindMethod(const char* signature)
{
  if (strcmp(signature, "static Benchmark.arguments(_,_,_,_)") == 0) return arguments;

  return NULL;
}
