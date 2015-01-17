#ifndef wren_core_h
#define wren_core_h

#include "wren_vm.h"

// This module defines the built-in classes and their native methods that are
// implemented directly in C code. Some languages try to implement as much of
// the core library itself in the primary language instead of in the host
// language.
//
// With Wren, we try to do as much of it in C as possible. Native methods are
// always faster than code written in Wren, and it minimizes startup time since
// we don't have to parse, compile, and execute Wren code.
//
// There is one limitation, though. Currently, native methods cannot call
// non-native ones. They can only be the top of the callstack, and immediately
// return. This makes it difficult to have native methods that rely on
// polymorphic behavior. For example, `io.write` should call `toString` on its
// argument, including user-defined `toString` methods on user-defined classes.

void wrenInitializeCore(WrenVM* vm);

typedef struct {
  // Set to true if the range was not valid, with the assumption that the error
  // was already set on the stack.
  bool invalidRange;

  // The index from which the range should start.
  int start;

  // The direction that the range is going; set to 1 if the range is increasing
  // from the [start] index or -1 if the range is decreasing from the [start]
  // index.
  int step;

  // Length of the range.
  int count;
} CalculatedRange;

#endif
