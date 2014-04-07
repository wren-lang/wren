#include "wren_io.h"

#if WREN_USE_LIB_IO

#include <stdio.h>
#include <time.h>

// This string literal is generated automatically from io.wren. Do not edit.
static const char* libSource =
"class IO {\n"
"  static print(obj) {\n"
"    IO.writeString_(obj.toString)\n"
"    IO.writeString_(\"\n\")\n"
"    return obj\n"
"  }\n"
"\n"
"  static write(obj) {\n"
"    IO.writeString_(obj.toString)\n"
"    return obj\n"
"  }\n"
"}\n";

static void ioWriteString(WrenVM* vm)
{
  const char* s = wrenGetArgumentString(vm, 1);
  // TODO: Check for null.
  printf("%s", s);
}

static void ioClock(WrenVM* vm)
{
  wrenReturnDouble(vm, (double)clock() / CLOCKS_PER_SEC);
}

void wrenLoadIOLibrary(WrenVM* vm)
{
  wrenInterpret(vm, "Wren IO library", libSource);
  wrenDefineStaticMethod(vm, "IO", "writeString_", 1, ioWriteString);
  wrenDefineStaticMethod(vm, "IO", "clock", 0, ioClock);
}

#endif
