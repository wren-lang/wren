#include "wren.h"

WrenForeignMethodFn slotsBindMethod(const char* signature);
void slotsBindClass(const char* className, WrenForeignClassMethods* methods);
