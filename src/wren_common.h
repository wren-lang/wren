#ifndef wren_common_h
#define wren_common_h

// TODO(bob): Use stdbool.h and `bool` for bools instead of int/1/0.

// This header contains macros and defines used across the entire Wren
// implementation. In particular, it contains "configuration" defines that
// control how Wren works. Some of these are only used while hacking on
// debugging Wren itself.
//
// This header is *not* intended to be included by code outside of Wren itself.

// TODO(bob): Prefix these with Wren and use #if instead of #ifdef. That way
// the flags can be set externally.

// Define this to stress test the GC. It will perform a collection before every
// allocation.
//#define DEBUG_GC_STRESS

// Define this to log memory operations.
//#define TRACE_MEMORY

#define NAN_TAGGING

// If this is set, the VM's interpreter loop uses computed gotos. See this for
// more: http://gcc.gnu.org/onlinedocs/gcc-3.1.1/gcc/Labels-as-Values.html
// TODO(bob): Automatically define this based on whether or not it's supported.
#define COMPUTED_GOTOS

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
