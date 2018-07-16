#include "wren.h"

WrenForeignMethodFn resolutionBindMethod(const char* signature);
void resolutionBindClass(const char* className, WrenForeignClassMethods* methods);
