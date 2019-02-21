#ifndef stat_h
#define stat_h

// Utilities to smooth over working with stat() in a cross-platform way.

// Windows doesn't define all of the Unix permission and mode flags by default,
// so map them ourselves.
#if defined(WIN32) || defined(WIN64)
  #include <sys\stat.h>

  // Map to Windows permission flags.
  #ifndef S_IRUSR
  #define S_IRUSR _S_IREAD
  #endif

  #ifndef S_IWUSR
  #define S_IWUSR _S_IWRITE
  #endif

  #ifndef S_ISREG
  #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
  #endif

  #ifndef S_ISDIR
  #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
  #endif

  // Not supported on Windows.
  #define O_SYNC 0
#endif

#endif
