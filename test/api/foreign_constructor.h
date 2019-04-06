#include "wren.h"

WrenForeignMethodFn foreignConstructorBindMethod(const char* signature);
void foreignConstructorBindClass(
    const char* className, WrenForeignClassMethods* methods);
