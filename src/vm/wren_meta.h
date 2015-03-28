#ifndef wren_meta_h
#define wren_meta_h

#include "wren.h"
#include "wren_common.h"
#include "wren_vm.h"

// This module defines the Meta class and its associated methods.
#if WREN_USE_LIB_META

void wrenLoadMetaLibrary(WrenVM* vm);

WrenForeignMethodFn wrenBindMetaForeignMethod(WrenVM* vm,
                                              const char* className,
                                              const char* signature);

#endif

#endif
