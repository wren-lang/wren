#ifndef io_h
#define io_h

#include "wren.h"

// Frees up any pending resources in use by the IO module.
//
// In particular, this closes down the stdin stream.
void ioShutdown();

#endif
