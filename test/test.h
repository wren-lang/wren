#pragma once
#ifndef WREN_TEST_H
#define WREN_TEST_H

#include <stdio.h>
#include <string.h>
#include "wren.h"

// Exit codes used by the wren binaries, following the BSD standard
//
// The interpreter was used with an incorrect number of arguments
#define WREN_EX_USAGE 64

// Compilation error
#define WREN_EX_DATAERR 65

// Runtime error
#define WREN_EX_SOFTWARE 70

// Cannot open input file
#define WREN_EX_NOINPUT 66

// I/O Error
#define WREN_EX_IOERR 74

// The maximum number of components in a path. We can't normalize a path that
// contains more than this number of parts. The number here assumes a max path
// length of 4096, which is common on Linux, and then assumes each component is
// at least two characters, "/", and a single-letter directory name.
#define MAX_COMPONENTS 2048

typedef struct {
  const char* start;
  const char* end;
} Slice;

// Categorizes what form a path is.
typedef enum
{
  // An absolute path, starting with "/" on POSIX systems, a drive letter on
  // Windows, etc.
  PATH_TYPE_ABSOLUTE,

  // An explicitly relative path, starting with "./" or "../".
  PATH_TYPE_RELATIVE,

  // A path that has no leading prefix, like "foo/bar".
  PATH_TYPE_SIMPLE,
} PathType;


typedef struct
{
  // Dynamically allocated array of characters.
  char* chars;

  // The number of characters currently in use in [chars], not including the
  // null terminator.
  size_t length;

  // Size of the allocated [chars] buffer.
  size_t capacity;
} Path;

//path helpers
  void ensureCapacity(Path* path, size_t capacity);
  void appendSlice(Path* path, Slice slice);
  void pathAppendString(Path* path, const char* string);
  void pathFree(Path* path);
  void pathDirName(Path* path);
  void pathRemoveExtension(Path* path);
  void pathAppendChar(Path* path, char c);
  void pathJoin(Path* path, const char* string);
  void pathNormalize(Path* path);
  char* pathToString(Path* path);
  PathType pathType(const char* path);
//file helpers
  char* readFile(const char* path);
  WrenLoadModuleResult readModule(WrenVM* vm, const char* module);
//vm helpers
  void vm_write(WrenVM* vm, const char* text);
  void reportError(WrenVM* vm, WrenErrorType type, const char* module, int line, const char* message);
  const char* resolveModule(WrenVM* vm, const char* importer, const char* module);
//main helpers
  bool isModuleAnAPITest(const char* module);
  WrenInterpretResult runFile(WrenVM* vm, const char* path);
  int handle_args(int argc, const char* argv[]);

#endif //WREN_TEST_H
