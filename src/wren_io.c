#include "wren_io.h"

#if WREN_USE_LIB_IO

#include <stdio.h>
#include <time.h>

// This string literal is generated automatically from io.wren. Do not edit.
static const char* libSource =
"class IO {\n"
"  static print {\n"
"    IO.writeString_(\"\n\")\n"
"  }\n"
"\n"
"  static print(obj) {\n"
"    IO.writeObject_(obj)\n"
"    IO.writeString_(\"\n\")\n"
"    return obj\n"
"  }\n"
"\n"
"  static print(a1, a2) {\n"
"    printList_([a1, a2])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3) {\n"
"    printList_([a1, a2, a3])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4) {\n"
"    printList_([a1, a2, a3, a4])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5) {\n"
"    printList_([a1, a2, a3, a4, a5])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6) {\n"
"    printList_([a1, a2, a3, a4, a5, a6])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15])\n"
"  }\n"
"\n"
"  static print(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) {\n"
"    printList_([a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16])\n"
"  }\n"
"\n"
"  static printList_(objects) {\n"
"    for (object in objects) IO.writeObject_(object)\n"
"    IO.writeString_(\"\n\")\n"
"  }\n"
"\n"
"  static write(obj) {\n"
"    IO.writeObject_(obj)\n"
"    return obj\n"
"  }\n"
"\n"
"  static writeObject_(obj) {\n"
"    var string = obj.toString\n"
"    if (string is String) {\n"
"      IO.writeString_(string)\n"
"    } else {\n"
"      IO.writeString_(\"[invalid toString]\")\n"
"    }\n"
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
