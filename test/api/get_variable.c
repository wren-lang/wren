#include <string.h>

#include "get_variable.h"

static void beforeDefined(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenGetVariable(vm, "./test/api/get_variable", "A", 0);
}

static void afterDefined(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenGetVariable(vm, "./test/api/get_variable", "A", 0);
}

static void afterAssigned(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenGetVariable(vm, "./test/api/get_variable", "A", 0);
}

static void otherSlot(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenSetSlotCount(fiber, 3);
  wrenGetVariable(vm, "./test/api/get_variable", "B", 2);
  
  // Move it into return position.
  const char* string = wrenGetSlotString(vm, 2);
  wrenSetSlotString(vm, 0, string);
}

static void otherModule(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenGetVariable(vm, "./test/api/get_variable_module", "Variable", 0);
}

WrenForeignMethodFn getVariableBindMethod(const char* signature)
{
  if (strcmp(signature, "static GetVariable.beforeDefined()") == 0) return beforeDefined;
  if (strcmp(signature, "static GetVariable.afterDefined()") == 0) return afterDefined;
  if (strcmp(signature, "static GetVariable.afterAssigned()") == 0) return afterAssigned;
  if (strcmp(signature, "static GetVariable.otherSlot()") == 0) return otherSlot;
  if (strcmp(signature, "static GetVariable.otherModule()") == 0) return otherModule;

  return NULL;
}
