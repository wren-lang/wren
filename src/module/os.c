#include "os.h"
#include "wren.h"

#include "uv.h"

#if __APPLE__
  #include "TargetConditionals.h"
#endif

#ifdef __APPLE__
  #define WREN_ON_POSIX 1
#elif __linux__
  #define WREN_ON_POSIX 1
#elif __unix__
  #define WREN_ON_POSIX 1
#elif defined(_POSIX_VERSION)
  #define WREN_ON_POSIX 1
#endif

int numArgs;
const char** args;

void osSetArguments(int argc, const char* argv[])
{
  numArgs = argc;
  args = argv;
}

void platformName(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  
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

void platformIsPosix(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
#ifdef WREN_ON_POSIX
  wrenSetSlotBool(vm, 0, true);
#else
  wrenSetSlotBool(vm, 0, false);
#endif
}

void processGetPid(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, (int)uv_os_getpid());
}

void processGetPPid(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  wrenSetSlotDouble(vm, 0, (int)uv_os_getppid());
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