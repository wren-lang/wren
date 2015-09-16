#ifndef io_h
#define io_h

#include "wren.h"

WrenForeignMethodFn ioBindForeign(
    WrenVM* vm, const char* className, bool isStatic, const char* signature);

#endif
