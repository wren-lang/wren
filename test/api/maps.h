#include "wren.h"

WrenForeignMethodFn mapsBindMethod(const char* signature);
void mapBindClass(
    const char* className, WrenForeignClassMethods* methods);
