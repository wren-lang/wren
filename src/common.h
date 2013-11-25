#ifndef wren_common_h
#define wren_common_h

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
