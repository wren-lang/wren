#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

// The maximum number of components in a path. We can't normalize a path that
// contains more than this number of parts. The number here assumes a max path
// length of 4096, which is common on Linux, and then assumes each component is
// at least two characters, "/", and a single-letter directory name.
#define MAX_COMPONENTS 2048

typedef struct {
  const char* start;
  const char* end;
} Slice;

static void ensureCapacity(Path* path, size_t capacity)
{
  // Capacity always needs to be one greater than the actual length to have
  // room for the null byte, which is stored in the buffer, but not counted in
  // the length. A zero-character path still needs a one-character array to
  // store the '\0'.
  capacity++;
  
  if (path->capacity >= capacity) return;
  
  // Grow by doubling in size.
  size_t newCapacity = 16;
  while (newCapacity < capacity) newCapacity *= 2;
  
  path->chars = (char*)realloc(path->chars, newCapacity);
  path->capacity = newCapacity;
}

static void appendSlice(Path* path, Slice slice)
{
  size_t length = slice.end - slice.start;
  ensureCapacity(path, path->length + length);
  memcpy(path->chars + path->length, slice.start, length);
  path->length += length;
  path->chars[path->length] = '\0';
}

Path* pathNew(const char* string)
{
  Path* path = (Path*)malloc(sizeof(Path));
  path->chars = (char*)malloc(1);
  path->chars[0] = '\0';
  path->length = 0;
  path->capacity = 0;

  pathAppendString(path, string);

  return path;
}

void pathFree(Path* path)
{
  if (path->chars) free(path->chars);
  free(path);
}

void pathDirName(Path* path)
{
  // Find the last path separator.
  for (size_t i = path->length - 1; i < path->length; i--)
  {
    if (path->chars[i] == '/')
    {
      path->length = i;
      path->chars[i] = '\0';
      return;
    }
  }

  // If we got here, there was no separator so it must be a single component.
  path->length = 0;
  path->chars[0] = '\0';
}

void pathRemoveExtension(Path* path)
{
  for (size_t i = path->length - 1; i < path->length; i--)
  {
    // If we hit a path separator before finding the extension, then the last
    // component doesn't have one.
    if (path->chars[i] == '/') return;
    
    if (path->chars[i] == '.')
    {
      path->length = i;
      path->chars[path->length] = '\0';
    }
  }
}

void pathJoin(Path* path, const char* string)
{
  if (path->length > 0 && path->chars[path->length - 1] != '/')
  {
    pathAppendChar(path, '/');
  }

  pathAppendString(path, string);
}


#ifdef _WIN32
static bool isDriveLetter(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
#endif

bool pathIsAbsolute(Path* path)
{
#ifdef _WIN32
  // Absolute path in the current drive.
  if (path->length >= 1 && path->chars[0] == '\\') return true;

  // Drive letter.
  if (path->length >= 3 &&
      isDriveLetter(path->chars[0]) &&
      path->chars[1] == ':' &&
      path->chars[2] == '\\')
  {
    return true;
  }

  // UNC path.
  return path->length >= 2 && path->chars[0] == '\\' && path->chars[1] == '\\';
#else
  // Otherwise, assume POSIX-style paths.
  return path->length >= 1 && path->chars[0] == '/';
#endif
}

void pathAppendChar(Path* path, char c)
{
  ensureCapacity(path, path->length + 1);
  path->chars[path->length++] = c;
  path->chars[path->length] = '\0';
}

void pathAppendString(Path* path, const char* string)
{
  Slice slice;
  slice.start = string;
  slice.end = string + strlen(string);
  appendSlice(path, slice);
}

Path* pathNormalize(Path* path)
{
  Path* result = pathNew("");
  
  // Split the path into components.
  Slice components[MAX_COMPONENTS];
  int numComponents = 0;
  
  char* start = path->chars;
  char* end = path->chars;

  // Split into parts and handle "." and "..".
  int leadingDoubles = 0;
  for (;;)
  {
    if (*end == '\0' || *end == '/')
    {
      // Add the current component.
      if (start != end)
      {
        size_t length = end - start;
        if (length == 1 && start[0] == '.')
        {
          // Skip "." components.
        }
        else if (length == 2 && start[0] == '.' && start[1] == '.')
        {
          // Walk out of directories on "..".
          if (numComponents > 0)
          {
            // Discard the previous component.
            numComponents--;
          }
          else
          {
            // Can't back out any further, so preserve the "..".
            leadingDoubles++;
          }
        }
        else
        {
          if (numComponents >= MAX_COMPONENTS)
          {
            fprintf(stderr, "Path cannot have more than %d path components.\n",
                    MAX_COMPONENTS);
            exit(1);
          }
          
          components[numComponents].start = start;
          components[numComponents].end = end;
          numComponents++;
        }
      }
      
      // Skip over separators.
      while (*end != '\0' && *end == '/') end++;
      
      start = end;
      if (*end == '\0') break;
    }
    
    end++;
  }
  
  // Preserve the absolute prefix, if any.
  bool needsSeparator = false;
  if (path->length > 0 && path->chars[0] == '/')
  {
    pathAppendChar(result, '/');
  }
  else
  {
    // Add any leading "..".
    for (int i = 0; i < leadingDoubles; i++)
    {
      if (needsSeparator) pathAppendChar(result, '/');
      pathAppendString(result, "..");
      needsSeparator = true;
    }
  }
  
  for (int i = 0; i < numComponents; i++)
  {
    if (needsSeparator) pathAppendChar(result, '/');
    appendSlice(result, components[i]);
    needsSeparator = true;
  }
  
  if (result->length == 0) pathAppendChar(result, '.');
  
  return result;
}

char* pathToString(Path* path)
{
  char* string = (char*)malloc(path->length + 1);
  memcpy(string, path->chars, path->length);
  string[path->length] = '\0';
  return string;
}
