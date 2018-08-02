#include <string.h>
#include <time.h>

#include "benchmark.h"

static void arguments(WrenFiber* fiber)
{
  double result = 0;

  result += wrenGetSlotDouble(fiber, 1);
  result += wrenGetSlotDouble(fiber, 2);
  result += wrenGetSlotDouble(fiber, 3);
  result += wrenGetSlotDouble(fiber, 4);

  wrenSetSlotDouble(fiber, 0, result);
}

const char* testScript =
"class Test {\n"
"  static method(a, b, c, d) { a + b + c + d }\n"
"}\n";

static void call(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  int iterations = (int)wrenGetSlotDouble(fiber, 1);
  
  // Since the VM is re-entrant, we can call from within this foreign method.
  
  wrenInterpret(vm, "main", testScript);
  
  WrenHandle* method = wrenMakeCallHandle(vm, "method(_,_,_,_)");
  
  wrenSetSlotCount(fiber, 1);
  wrenGetVariable(vm, "main", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(fiber, 0);
  
  double startTime = (double)clock() / CLOCKS_PER_SEC;
  
  double result = 0;
  for (int i = 0; i < iterations; i++)
  {
    wrenSetSlotCount(fiber, 5);
    wrenSetSlotHandle(fiber, 0, testClass);
    wrenSetSlotDouble(fiber, 1, 1.0);
    wrenSetSlotDouble(fiber, 2, 2.0);
    wrenSetSlotDouble(fiber, 3, 3.0);
    wrenSetSlotDouble(fiber, 4, 4.0);
    
    wrenCall(fiber, method);
    
    result += wrenGetSlotDouble(fiber, 0);
  }
  
  double elapsed = (double)clock() / CLOCKS_PER_SEC - startTime;
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, method);
  
  if (result == (1.0 + 2.0 + 3.0 + 4.0) * iterations)
  {
    wrenSetSlotDouble(fiber, 0, elapsed);
  }
  else
  {
    // Got the wrong result.
    wrenSetSlotBool(fiber, 0, false);
  }
}

WrenForeignMethodFn benchmarkBindMethod(const char* signature)
{
  if (strcmp(signature, "static Benchmark.arguments(_,_,_,_)") == 0) return arguments;
  if (strcmp(signature, "static Benchmark.call(_)") == 0) return call;

  return NULL;
}
