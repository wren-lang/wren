#ifndef modules_h
#define modules_h

// This wires up all of the foreign classes and methods defined by the built-in
// modules bundled with the CLI.

#include "wren.h"

// Returns the source for built-in module [name].
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
