#include <stdio.h>
#include <string.h>

#include "wren.h"

void resetStackAfterCallAbortRunTests(WrenVM* vm)
{
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  
  wrenSetSlotCount(fiber, 1);
  wrenGetVariable(vm, "./test/api/reset_stack_after_call_abort", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(fiber, 0);
  
  WrenHandle* abortFiber = wrenMakeCallHandle(vm, "abortFiber()");
  WrenHandle* afterConstruct = wrenMakeCallHandle(vm, "afterAbort(_,_)");
  
  wrenSetSlotCount(fiber, 1);
  wrenSetSlotHandle(fiber, 0, testClass);
  wrenCall(fiber, abortFiber);

  // Recreate a new fiber here
  fiber = wrenGetCurrentFiber(vm);
  wrenSetSlotCount(fiber, 3);
  wrenSetSlotHandle(fiber, 0, testClass);
  wrenSetSlotDouble(fiber, 1, 1.0);
  wrenSetSlotDouble(fiber, 2, 2.0);
  wrenCall(fiber, afterConstruct);
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, abortFiber);
  wrenReleaseHandle(vm, afterConstruct);
}
