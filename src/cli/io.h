#ifndef io_h
#define io_h

#include "wren.h"

// Simple IO functions.

// Reads the contents of the file at [path] and returns it as a heap allocated
// string.
//
// Returns `NULL` if the path could not be found. Exits if it was found but
// could not be read.
char* readFile(const char* path);

// Sets the root directory that modules are resolved relative to.
void setRootDirectory(const char* path);

// Attempts to read the source for [module] relative to the current root
// directory.
//
// Returns it if found, or NULL if the module could not be found. Exits if the
// module was found but could not be read.
char* readModule(WrenVM* vm, const char* module);

#endif
