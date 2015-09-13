#ifndef timer_h
#define timer_h

#include "wren.h"

WrenForeignMethodFn timerBindForeign(
    WrenVM* vm, const char* className, bool isStatic, const char* signature);

#endif
