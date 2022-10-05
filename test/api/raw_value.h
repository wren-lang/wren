#include "wren.h"


WrenForeignMethodFn rawValueBindMethod(const char* signature);
void rawValueBindClass(
    const char* className, WrenForeignClassMethods* methods);
