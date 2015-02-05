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
#define WREN_NAN_TAGGING 1
#endif

// If true, the VM's interpreter loop uses computed gotos. See this for more:
// http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// Enabling this speeds up the main dispatch loop a bit, but requires compiler
// support.
//
// Defaults to on on supported compilers.
#ifndef WREN_COMPUTED_GOTO
  #ifdef _MSC_VER
    // No computed gotos in Visual Studio.
    #define WREN_COMPUTED_GOTO 0
  #else
    #define WREN_COMPUTED_GOTO 1
  #endif
#endif

// If true, loads the "IO" class in the standard library.
//
// Defaults to on.
#ifndef WREN_USE_LIB_IO
#define WREN_USE_LIB_IO 1
#endif

// These flags are useful for debugging and hacking on Wren itself. They are not
// intended to be used for production code. They default to off.

// Set this to true to stress test the GC. It will perform a collection before
// every allocation. This is useful to ensure that memory is always correctly
// reachable.
#define WREN_DEBUG_GC_STRESS 0

// Set this to true to log memory operations as they occur.
#define WREN_DEBUG_TRACE_MEMORY 0

// Set this to true to log garbage collections as they occur.
#define WREN_DEBUG_TRACE_GC 0

// Set this to true to print out the compiled bytecode of each function.
#define WREN_DEBUG_DUMP_COMPILED_CODE 0

// Set this to trace each instruction as it's executed.
#define WREN_DEBUG_TRACE_INSTRUCTIONS 0

// The maximum number of module-level variables that may be defined at one time.
// This limitation comes from the 16 bits used for the arguments to
// `CODE_LOAD_MODULE_VAR` and `CODE_STORE_MODULE_VAR`.
#define MAX_MODULE_VARS 65536

// The maximum number of arguments that can be passed to a method. Note that
// this limitation is hardcoded in other places in the VM, in particular, the
// `CODE_CALL_XX` instructions assume a certain maximum number.
#define MAX_PARAMETERS 16

// The maximum name of a method, not including the signature. This is an
// arbitrary but enforced maximum just so we know how long the method name
// strings need to be in the parser.
#define MAX_METHOD_NAME 64

// The maximum length of a method signature. This includes the name, and the
// extra spaces added to handle arity, and another byte to terminate the string.
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + MAX_PARAMETERS + 1)

// The maximum length of an identifier. The only real reason for this limitation
// is so that error messages mentioning variables can be stack allocated.
#define MAX_VARIABLE_NAME 64

// The maximum number of fields a class can have, including inherited fields.
// This is explicit in the bytecode since `CODE_CLASS` and `CODE_SUBCLASS` take
// a single byte for the number of fields. Note that it's 255 and not 256
// because creating a class takes the *number* of fields, not the *highest
// field index*.
#define MAX_FIELDS 255

// The Microsoft compiler does not support the "inline" modifier when compiling
// as plain C.
#if defined( _MSC_VER ) && !defined(__cplusplus)
#define inline _inline
#endif

// This is used to clearly mark flexible-sized arrays that appear at the end of
// some dynamically-allocated structs, known as the "struct hack".
#if __STDC_VERSION__ >= 199901L
// In C99, a flexible array member is just "[]".
#define FLEXIBLE_ARRAY
#else
// Elsewhere, use a zero-sized array. It's technically undefined behavior, but
// works reliably in most known compilers.
#define FLEXIBLE_ARRAY 0
#endif

// Assertions are used to validate program invariants. They indicate things the
// program expects to be true about its internal state during execution. If an
// assertion fails, there is a bug in Wren.
//
// Assertions add significant overhead, so are only enabled in debug builds.
#ifdef DEBUG

#include <stdio.h>

#define ASSERT(condition, message) \
    do \
    { \
      if (!(condition)) \
      { \
        fprintf(stderr, "[%s:%d] Assert failed in %s(): %s\n", \
            __FILE__, __LINE__, __func__, message); \
        abort(); \
      } \
    } \
    while(0)

// Assertion to indicate that the given point in the code should never be
// reached.
#define UNREACHABLE() \
    do \
    { \
      fprintf(stderr, "This line should not be reached.\n"); \
      abort(); \
    } \
    while (0)

#else

#define ASSERT(condition, message) do { } while (0)
#define UNREACHABLE() do { } while (0)

#endif

#endif
