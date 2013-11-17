#ifndef wren_common_h
#define wren_common_h

// Define this to stress test the GC. It will perform a collection before every
// allocation.
#define DEBUG_GC_STRESS

// Define this to log memory operations.
//#define TRACE_MEMORY

#define NAN_TAGGING

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
