#include "wren.h"

void resetStackAfterForeignConstructBindClass(
    const char* className, WrenForeignClassMethods* methods);
void resetStackAfterForeignConstructRunTests(WrenVM* vm);