#ifndef path_h
#define path_h

// Path manipulation functions.

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

PathType pathType(const char* path);

// Creates a new empty path.
Path* pathNew(const char* string);

// Releases the method associated with [path].
void pathFree(Path* path);

// Strips off the last component of the path name.
void pathDirName(Path* path);

// Strips off the file extension from the last component of the path.
void pathRemoveExtension(Path* path);

// Appends [string] to [path].
void pathJoin(Path* path, const char* string);

// Appends [c] to the path, growing the buffer if needed.
void pathAppendChar(Path* path, char c);

// Appends [string] to the path, growing the buffer if needed.
void pathAppendString(Path* path, const char* string);

// Simplifies the path string as much as possible.
//
// Applies and removes any "." or ".." components, collapses redundant "/"
// characters, and normalizes all path separators to "/".
void pathNormalize(Path* path);

// Allocates a new string exactly the right length and copies this path to it.
char* pathToString(Path* path);

#endif
