#ifndef wren_meta_h
#define wren_meta_h

#include "wren_common.h"

#include "wren.h"

// This module defines the Meta class and its associated methods.
#if WREN_USE_META_MODULE

void wrenLoadMetaModule(WrenVM* vm);

#endif

#endif
