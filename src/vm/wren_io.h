#ifndef wren_io_h
#define wren_io_h

#include "wren.h"
#include "wren_common.h"

// This module defines the IO class and its associated methods. They are
// implemented using the C standard library.
#if WREN_USE_LIB_IO

void wrenLoadIOLibrary(WrenVM* vm);

WrenForeignMethodFn wrenBindIOForeignMethod(WrenVM* vm, const char* className,
                                            const char* signature);

#endif

#endif
