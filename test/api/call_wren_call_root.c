#include <stdio.h>
#include <string.h>

#include "wren.h"

int callWrenCallRootRunTests(WrenVM* vm)
{
  int exitCode = 0;
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "./test/api/call_wren_call_root", "Test", 0);
  WrenHandle* testClass = wrenGetSlotHandle(vm, 0);

  WrenHandle* run = wrenMakeCallHandle(vm, "run()");

  wrenEnsureSlots(vm, 1);
  wrenSetSlotHandle(vm, 0, testClass);
  WrenInterpretResult result = wrenCall(vm, run);
  if (result == WREN_RESULT_RUNTIME_ERROR)
  {
    exitCode = 70;
  }
  else
  {
    printf("Missing runtime error.\n");
  }

  wrenReleaseHandle(vm, testClass);
  wrenReleaseHandle(vm, run);
  return exitCode;
}
