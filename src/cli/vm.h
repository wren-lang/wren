#ifndef vm_h
#define vm_h

#include "wren.h"

// Creates a new Wren VM with the CLI's module loader and other configuration.
WrenVM* createVM();

// Executes the Wren script at [path] in a new VM.
//
// Exits if the script failed or could not be loaded.
void runFile(const char* path);

// Adds additional callbacks to use when binding foreign members from Wren.
//
// Used by the API test executable to let it wire up its own foreign functions.
// This must be called before calling [createVM()].
void setForeignCallbacks(WrenBindForeignMethodFn bindMethod,
                         WrenBindForeignClassFn bindClass);

#endif
