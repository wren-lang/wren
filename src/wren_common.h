#ifndef wren_common_h
#define wren_common_h

// This header contains macros and defines used across the entire Wren
// implementation. In particular, it contains "configuration" defines that
// control how Wren works. Some of these are only used while hacking on
// debugging Wren itself.
//
// This header is *not* intended to be included by code outside of Wren itself.

// These flags let you control some details of the interpreter's implementation.
// Usually they trade-off a bit of portability for speed. They default to the
// most efficient behavior.

// If true, then Wren will use a NaN-tagged double for its core value
// representation. Otherwise, it will use a larger more conventional struct.
// The former is significantly faster and more compact. The latter is useful for
// debugging and may be more portable.
//
// Defaults to on.
#ifndef WREN_NAN_TAGGING
#define WREN_NAN_TAGGING true
#endif

// If true, the VM's interpreter loop uses computed gotos. See this for more:
// http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// Enabling this speeds up the main dispatch loop a bit, but requires compiler
// support.
//
// Defaults to on.
#ifndef WREN_COMPUTED_GOTO
#define WREN_COMPUTED_GOTO true
#endif

// These flags are useful for debugging and hacking on Wren itself. They are not
// intended to be used for production code. They default to off.

// Set this to true to stress test the GC. It will perform a collection before
// every allocation. This is useful to ensure that memory is always correctly
// pinned.
#define WREN_DEBUG_GC_STRESS false

// Set this to true to log memory operations as they occur.
#define WREN_TRACE_MEMORY false

// Set this to true to log garbage collections as they occur.
#define WREN_TRACE_GC false

// The maximum number of arguments that can be passed to a method. Note that
// this limtation is hardcoded in other places in the VM, in particular, the
// `CODE_CALL_XX` instructions assume a certain maximum number.
#define MAX_PARAMETERS (16)

// The maximum name of a method, not including the signature. This is an
// arbitrary but enforced maximum just so we know how long the method name
// strings need to be in the parser.
#define MAX_METHOD_NAME (64)

// The maximum length of a method signature. This includes the name, and the
// extra spaces added to handle arity, and another byte to terminate the string.
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + MAX_PARAMETERS + 1)

// TODO: Don't hardcode this.
#define MAX_SYMBOLS 256

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

// Assertion to indicate that the given point in the code should never be
// reached.
#define UNREACHABLE() ASSERT(false, "This line should not be reached.");

#endif
