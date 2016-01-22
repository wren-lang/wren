#include "process.h"
#include "wren.h"

int numArgs;
const char** args;

void processSetArguments(int argc, const char* argv[])
{
  numArgs = argc;
  args = argv;
}

void processAllArguments(WrenVM* vm)
{
  wrenEnsureSlots(vm, 2);
  wrenSetSlotNewList(vm, 0);

  for (int i = 0; i < numArgs; i++)
  {
    wrenSetSlotString(vm, 1, args[i]);
    wrenInsertInList(vm, 0, -1, 1);
  }
}