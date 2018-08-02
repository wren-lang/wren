#include <stdio.h>
#include <string.h>

#include "reset_stack_after_foreign_construct.h"

static void counterAllocate(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  double* counter = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double));
  *counter = wrenGetSlotDouble(vm, 1);
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
  wrenSetSlotCount(vm, 1);
  wrenGetVariable(vm,
      "./test/api/reset_stack_after_foreign_construct", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(vm, 0);
  
  WrenHandle* callConstruct = wrenMakeCallHandle(vm, "callConstruct()");
  WrenHandle* afterConstruct = wrenMakeCallHandle(vm, "afterConstruct(_,_)");
  
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenCall(vm, callConstruct);

  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, testClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, afterConstruct);
  
  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, callConstruct);
  wrenReleaseHandle(vm, afterConstruct);
}
