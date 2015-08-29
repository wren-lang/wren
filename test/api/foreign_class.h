#include "wren.h"

WrenForeignMethodFn foreignClassBindMethod(const char* signature);
void foreignClassBindClass(
    const char* className, WrenForeignClassMethods* methods);
