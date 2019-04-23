#include <stdio.h>
#include <string.h>

#include "wren.h"

static void api(WrenVM *vm) {
  // Grow the slot array. This should trigger the stack to be moved.
  wrenEnsureSlots(vm, 10);
  wrenSetSlotNewList(vm, 0);
  
  for (int i = 1; i < 10; i++)
  {
    wrenSetSlotDouble(vm, i, i);
    wrenInsertInList(vm, 0, -1, i);
  }
}

static void api2(WrenVM *vm) {
  // This should trigger a stack reallocation again.
  wrenEnsureSlots(vm, 40);
  // Return a number.
  wrenSetSlotDouble(vm, 0, 42.0);
}

WrenForeignMethodFn callCallsForeignBindMethod(const char* signature)
{
  if (strcmp(signature, "static CallCallsForeign.api()") == 0) return api;
  if (strcmp(signature, "static CallCallsForeign.api2()") == 0) return api2;
  
  return NULL;
}

void callCallsForeignRunTests(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "./test/api/call_calls_foreign", "CallCallsForeign", 0);
  WrenHandle* apiClass = wrenGetSlotHandle(vm, 0);
  WrenHandle *call = wrenMakeCallHandle(vm, "call(_)");
  
  wrenEnsureSlots(vm, 2);
  wrenSetSlotHandle(vm, 0, apiClass);
  wrenSetSlotString(vm, 1, "parameter");

  printf("slots before %d\n", wrenGetSlotCount(vm));
  wrenCall(vm, call);
  
  // We should have a single slot count for the return.
  printf("slots after %d\n", wrenGetSlotCount(vm));

  WrenHandle *call2 = wrenMakeCallHandle(vm, "call2()");

  wrenSetSlotHandle(vm, 0, apiClass);
  wrenCall(vm, call2);
  // Checks the return type of call2().
  // This should print 1, the value of WREN_TYPE_NUM.
  // If the stack is corrupted, it may print 6.
  printf("return type %d\n", wrenGetSlotType(vm, 0));
  
  wrenReleaseHandle(vm, call);
  wrenReleaseHandle(vm, call2);
  wrenReleaseHandle(vm, apiClass);
}
