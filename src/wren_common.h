#ifndef wren_common_h
#define wren_common_h

// TODO(bob): Use stdbool.h and `bool` for bools instead of int/1/0.

// This header contains macros and defines used across the entire Wren
// implementation. In particular, it contains "configuration" defines that
// control how Wren works. Some of these are only used while hacking on
// debugging Wren itself.
//
// This header is *not* intended to be included by code outside of Wren itself.

// These flags let you control some details of the interpreter's implementation.
// Usually they trade-off a bit of portability for speed. They default to the
// most efficient behavior.

// If non-zero, then Wren will use a NaN-tagged double for its core value
// representation. Otherwise, it will use a larger more conventional struct.
// The former is significantly faster and more compact. The latter is useful for
// debugging and may be more portable.
//
// Defaults to on.
#ifndef WREN_NAN_TAGGING
#define WREN_NAN_TAGGING (1)
#endif

// If non-zero, the VM's interpreter loop uses computed gotos. See this for
// more: http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// Enabling this speeds up the main dispatch loop a bit, but requires compiler
// support.
//
// Defaults to on.
#ifndef WREN_COMPUTED_GOTO
#define WREN_COMPUTED_GOTO (1)
#endif

// These flags are useful for debugging and hacking on Wren itself. They are not
// intended to be used for production code. They default to off.

// Set this to non-zero to stress test the GC. It will perform a collection
// before every allocation. This is useful to ensure that memory is always
// correctly pinned.
#define WREN_DEBUG_GC_STRESS (0)

// Set this to non-zero to log memory operations as they occur.
#define WREN_TRACE_MEMORY (0)

// Assertions are used to validate program invariants. They indicate things the
// program expects to be true about its internal state during execution. If an
// assertion fails, there is a bug in Wren.
//
// Assertions add significant overhead, so are only enabled in debug builds.
#ifdef DEBUG

#define ASSERT(condition, message)                                   \
  if (!(condition)) {                                                \
    printf("ASSERT FAIL " __FILE__ ":%d - %s\n", __LINE__, message); \
    abort();                                                         \
  }

#else

#define ASSERT(condition, message) ;

#endif

#endif
