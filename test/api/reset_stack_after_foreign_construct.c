#include <stdio.h>
#include <string.h>

#include "reset_stack_after_foreign_construct.h"

static void counterAllocate(WrenFiber* fiber)
{
  double* counter = (double*)wrenSetSlotNewForeign(fiber, 0, 0, sizeof(double));
  *counter = wrenGetSlotDouble(fiber, 1);
}

void resetStackAfterForeignConstructBindClass(
    const char* className, WrenForeignClassMethods* methods)
{
  if (strcmp(className, "ResetStackForeign") == 0)
  {
    methods->allocate = counterAllocate;
    return;
  }
}

void resetStackAfterForeignConstructRunTests(WrenVM* vm)
{
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  
  wrenSetSlotCount(fiber, 1);
  wrenGetVariable(vm,
      "./test/api/reset_stack_after_foreign_construct", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(fiber, 0);
  
  WrenHandle* callConstruct = wrenMakeCallHandle(vm, "callConstruct()");
  WrenHandle* afterConstruct = wrenMakeCallHandle(vm, "afterConstruct(_,_)");
  
  wrenSetSlotCount(fiber, 1);
  wrenSetSlotHandle(fiber, 0, testClass);
  wrenCall(fiber, callConstruct);

  wrenSetSlotCount(fiber, 3);
  wrenSetSlotHandle(fiber, 0, testClass);
  wrenSetSlotDouble(fiber, 1, 1.0);
  wrenSetSlotDouble(fiber, 2, 2.0);
  wrenCall(fiber, afterConstruct);
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, callConstruct);
  wrenReleaseHandle(vm, afterConstruct);
}
