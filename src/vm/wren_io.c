#include "wren_io.h"

#if WREN_USE_LIB_IO

#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: This is an arbitrary limit Do something smarter.
#define MAX_READ_LEN 1024

// This string literal is generated automatically from io.wren. Do not edit.
static const char* ioLibSource =
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
"  static read(prompt) {\n"
"    if (!(prompt is String)) Fiber.abort(\"Prompt must be a string.\")\n"
"    IO.write(prompt)\n"
"    return IO.read\n"
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
"\n"
"  foreign static writeString_(string)\n"
"  foreign static clock\n"
"  foreign static time\n"
"  foreign static read\n"
"}\n";

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

  if (result != NULL) {
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
  wrenInterpret(vm, "", ioLibSource);
}

WrenForeignMethodFn wrenBindIOForeignMethod(WrenVM* vm, const char* className,
                                            const char* signature)
{
  if (strcmp(className, "IO") != 0) return NULL;
  
  if (strcmp(signature, "writeString_(_)") == 0) return ioWriteString;
  if (strcmp(signature, "clock") == 0) return ioClock;
  if (strcmp(signature, "time") == 0) return ioTime;
  if (strcmp(signature, "read") == 0) return ioRead;
  return NULL;
}

#endif
