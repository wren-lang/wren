#include "wren.h"

void resetStackAfterForeignConstructBindClass(
    const char* className, WrenForeignClassMethods* methods);
int resetStackAfterForeignConstructRunTests(WrenVM* vm);