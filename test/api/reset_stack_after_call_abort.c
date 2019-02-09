#include <stdio.h>
#include <string.h>

#include "wren.h"

void resetStackAfterCallAbortRunTests(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "./test/api/reset_stack_after_call_abort", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(vm, 0);
  
  WrenHandle* abortFiber = wrenMakeCallHandle(vm, "abortFiber()");
  WrenHandle* afterAbort = wrenMakeCallHandle(vm, "afterAbort(_,_)");
  
  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenCall(vm, abortFiber);

  wrenEnsureSlots(vm, 3);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, afterAbort);
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, abortFiber);
  wrenReleaseHandle(vm, afterAbort);
}
