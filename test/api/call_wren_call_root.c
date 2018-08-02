#include <stdio.h>
#include <string.h>

#include "wren.h"
#include "vm.h"

void callWrenCallRootRunTests(WrenVM* vm)
{
  WrenFiber* fiber = wrenGetCurrentFiber(vm);
  wrenSetSlotCount(fiber, 1);
  wrenGetVariable(vm, "./test/api/call_wren_call_root", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(vm, 0);

  WrenHandle* run = wrenMakeCallHandle(vm, "run()");

  wrenSetSlotCount(fiber, 1);
  wrenSetSlotHandle(vm, 0, testClass);
  WrenInterpretResult result = wrenCall(fiber, run);
  if (result == WREN_RESULT_RUNTIME_ERROR)
  {
    setExitCode(70);
  }
  else
  {
    printf("Missing runtime error.\n");
  }

  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, run);
}
