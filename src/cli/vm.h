#ifndef vm_h
#define vm_h

#include "uv.h"
#include "wren.h"

// Executes the Wren script at [path] in a new VM.
//
// If [bindForeign] is not `NULL`, it will be called to register any foreign
// methods that the CLI itself doesn't handle.
void runFile(const char* path, WrenBindForeignMethodFn bindForeign);

// Runs the Wren interactive REPL.
int runRepl();

// Gets the currently running VM.
WrenVM* getVM();

// Gets the event loop the VM is using.
uv_loop_t* getLoop();

#endif
