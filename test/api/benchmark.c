#include <string.h>
#include <time.h>

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

const char* testScript =
"class Test {\n"
"  static method(a, b, c, d) { a + b + c + d }\n"
"}\n";

static void call(WrenVM* vm)
{
  int iterations = (int)wrenGetSlotDouble(vm, 1);
  
  // Since the VM is not re-entrant, we can't call from within this foreign
  // method. Instead, make a new VM to run the call test in.
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  WrenVM* otherVM = wrenNewVM(&config);
  
  wrenInterpret(otherVM, testScript);
  
  WrenValue* method = wrenGetMethod(otherVM, "main", "Test", "method(_,_,_,_)");
  
  double startTime = (double)clock() / CLOCKS_PER_SEC;
  
  double result = 0;
  for (int i = 0; i < iterations; i++)
  {
    WrenValue* resultValue;
    wrenCall(otherVM, method, &resultValue, "dddd", 1.0, 2.0, 3.0, 4.0);
    result += wrenGetValueDouble(otherVM, resultValue);
    wrenReleaseValue(otherVM, resultValue);
  }
  
  double elapsed = (double)clock() / CLOCKS_PER_SEC - startTime;
  
  wrenReleaseValue(otherVM, method);
  wrenFreeVM(otherVM);
  
  if (result == (1.0 + 2.0 + 3.0 + 4.0) * iterations)
  {
    wrenSetSlotDouble(vm, 0, elapsed);
  }
  else
  {
    // Got the wrong result.
    wrenSetSlotBool(vm, 0, false);
  }
}

WrenForeignMethodFn benchmarkBindMethod(const char* signature)
{
  if (strcmp(signature, "static Benchmark.arguments(_,_,_,_)") == 0) return arguments;
  if (strcmp(signature, "static Benchmark.call(_)") == 0) return call;

  return NULL;
}
