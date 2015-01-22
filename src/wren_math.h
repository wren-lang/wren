#ifndef wren_math_h
#define wren_math_h

#include "wren.h"
#include "wren_common.h"

// This module defines the Ramdp, class and its associated methods. The RNG is
// based upon "xorshift".
#if WREN_USE_LIB_MATH

void wrenLoadMathLibrary(WrenVM* vm);

#endif

#endif
