#include "bytecode_loader.h"
#include "bytecode_test.h"

#include <string.h>

extern bool bytecodeEquivalenceRunTests(WrenVM* vm);
extern bool bytecodeRejectionRunTests(WrenVM* vm);
extern bool bytecodeAPIRunTests(WrenVM* vm);
extern bool bytecodeFormatRunTests(WrenVM* vm);

static void runTests(WrenVM* vm)
{
  bool ok = true;
  ok = bytecodeEquivalenceRunTests(vm) && ok;
  ok = bytecodeRejectionRunTests(vm) && ok;
  ok = bytecodeAPIRunTests(vm) && ok;
  ok = bytecodeFormatRunTests(vm) && ok;
  wrenSetSlotBool(vm, 0, ok);
}

WrenForeignMethodFn bytecodeLoaderBindMethod(const char* signature)
{
  if (strcmp(signature, "static BytecodeLoader.runTests()") == 0) return runTests;
  return NULL;
}
