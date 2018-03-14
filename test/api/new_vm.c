#include <string.h>

#include "new_vm.h"

static void nullConfig(WrenVM* vm)
{
  WrenVM* otherVM = wrenNewVM(NULL);
  
  // We should be able to execute code.
  WrenError result = wrenInterpret(otherVM, "1 + 2");
  wrenSetSlotBool(vm, 0, result == WREN_ERROR_NOERROR);
  
  wrenFreeVM(otherVM);
}

WrenForeignMethodFn newVMBindMethod(const char* signature)
{
  if (strcmp(signature, "static VM.nullConfig()") == 0) return nullConfig;

  return NULL;
}
