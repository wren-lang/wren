#include "os.h"
#include "wren.h"

#if __APPLE__
  #include "TargetConditionals.h"
#endif

int numArgs;
const char** args;

void osSetArguments(int argc, const char* argv[])
{
  numArgs = argc;
  args = argv;
}

void platformName(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenSetSlotCount(fiber, 1);
  
  #ifdef _WIN32
    wrenSetSlotString(vm, 0, "Windows");
  #elif __APPLE__
    #if TARGET_IPHONE_SIMULATOR || TARGET_OS_IPHONE
      wrenSetSlotString(vm, 0, "iOS");
    #elif TARGET_OS_MAC
      wrenSetSlotString(vm, 0, "OS X");
    #else
      wrenSetSlotString(vm, 0, "Unknown");
    #endif
  #elif __linux__
    wrenSetSlotString(vm, 0, "Linux");
  #elif __unix__
    wrenSetSlotString(vm, 0, "Unix");
  #elif defined(_POSIX_VERSION)
    wrenSetSlotString(vm, 0, "POSIX");
  #else
    wrenSetSlotString(vm, 0, "Unknown");
  #endif
}

void platformIsPosix(WrenFiber* fiber)
{
  wrenSetSlotCount(fiber, 1);
  
  #ifdef _WIN32
    wrenSetSlotBool(fiber, 0, false);
  #elif __APPLE__
    wrenSetSlotBool(fiber, 0, true);
  #elif __linux__
    wrenSetSlotBool(fiber, 0, true);
  #elif __unix__
    wrenSetSlotBool(fiber, 0, true);
  #elif defined(_POSIX_VERSION)
    wrenSetSlotBool(fiber, 0, true);
  #else
    wrenSetSlotBool(fiber, 0, false);
  #endif
}

void processAllArguments(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  wrenSetSlotCount(fiber, 2);
  wrenSetSlotNewList(fiber, 0);

  for (int i = 0; i < numArgs; i++)
  {
    wrenSetSlotString(vm, 1, args[i]);
    wrenInsertInList(fiber, 0, -1, 1);
  }
}
