#include "wren_io.h"

#if WREN_USE_LIB_IO

#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: This is an arbitrary limit. Do something smarter.
#define MAX_READ_LEN 1024

#include "wren_io.wren.inc"

static void ioWriteString(WrenVM* vm)
{
  const char* s = wrenGetArgumentString(vm, 1);
  // TODO: Check for null.
  printf("%s", s);
}

static void ioRead(WrenVM* vm)
{
  char buffer[MAX_READ_LEN];
  char* result = fgets(buffer, MAX_READ_LEN, stdin);

  if (result != NULL)
  {
    wrenReturnString(vm, buffer, (int)strlen(buffer));
  }
}

static void ioClock(WrenVM* vm)
{
  wrenReturnDouble(vm, (double)clock() / CLOCKS_PER_SEC);
}

static void ioTime(WrenVM* vm)
{
  wrenReturnDouble(vm, (double)time(NULL));
}

void wrenLoadIOLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "", ioModuleSource);
}

WrenForeignMethodFn wrenBindIOForeignMethod(WrenVM* vm, const char* className,
                                            const char* signature)
{
  if (strcmp(className, "IO") != 0) return NULL;

  if (strcmp(signature, "writeString_(_)") == 0) return ioWriteString;
  if (strcmp(signature, "clock") == 0) return ioClock;
  if (strcmp(signature, "time") == 0) return ioTime;
  if (strcmp(signature, "read()") == 0) return ioRead;
  return NULL;
}

#endif
