#ifndef wren_debug_h
#define wren_debug_h

#include "wren_value.h"
#include "wren_vm.h"

void wrenDebugPrintStackTrace(WrenVM* vm, ObjFiber* fiber);
int wrenDebugPrintInstruction(WrenVM* vm, ObjFn* fn, int i);
void wrenDebugPrintCode(WrenVM* vm, ObjFn* fn);
void wrenDebugPrintStack(ObjFiber* fiber);

#endif
