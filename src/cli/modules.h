#ifndef modules_h
#define modules_h

#include "wren.h"

char* readBuiltInModule(const char* module);

// Looks up a foreign method in a built-in module.
//
// Returns `NULL` if [moduleName] is not a built-in module.
WrenForeignMethodFn bindBuiltInForeignMethod(
    WrenVM* vm, const char* moduleName, const char* className, bool isStatic,
    const char* signature);

// Binds foreign classes declared in a built-in modules.
WrenForeignClassMethods bindBuiltInForeignClass(
    WrenVM* vm, const char* moduleName, const char* className);

#endif
