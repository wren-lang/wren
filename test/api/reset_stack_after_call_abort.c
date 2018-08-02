#include <stdio.h>
#include <string.h>

#include "wren.h"

void resetStackAfterCallAbortRunTests(WrenVM* vm)
{
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  
  wrenSetSlotCount(vm, 1);
  wrenGetVariable(vm, "./test/api/reset_stack_after_call_abort", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(vm, 0);
  
  WrenHandle* abortFiber = wrenMakeCallHandle(vm, "abortFiber()");
  WrenHandle* afterConstruct = wrenMakeCallHandle(vm, "afterAbort(_,_)");
  
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenCall(fiber, abortFiber);

  // Recreate a new fiber here
  fiber = wrenGetCurrentFiber(vm);
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(fiber, afterConstruct);
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, abortFiber);
  wrenReleaseHandle(vm, afterConstruct);
}
