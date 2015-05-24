#ifndef vm_h
#define vm_h

#include "wren.h"

// Creates a new Wren VM with the CLI's module loader and other configuration.
WrenVM* createVM(WrenBindForeignMethodFn bindForeign);

// Executes the Wren script at [path] in a new VM.
//
// Exits if the script failed or could not be loaded.
void runFile(WrenBindForeignMethodFn bindForeign, const char* path);

#endif
