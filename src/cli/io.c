#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"

char const* rootDirectory = NULL;

// Reads the contents of the file at [path] and returns it as a heap allocated
// string.
//
// Returns `NULL` if the path could not be found. Exits if it was found but
// could not be read.
char* readFile(const char* path)
{
  FILE* file = fopen(path, "rb");
  if (file == NULL) return NULL;

  // Find out how big the file is.
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // Allocate a buffer for it.
  char* buffer = (char*)malloc(fileSize + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  // Read the entire file.
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize)
  {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(74);
  }

  // Terminate the string.
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

void setRootDirectory(const char* path)
{
  rootDirectory = path;
}

char* readModule(WrenVM* vm, const char* module)
{
  // The module path is relative to the root directory and with ".wren".
  size_t rootLength = rootDirectory == NULL ? 0 : strlen(rootDirectory);
  size_t moduleLength = strlen(module);
  size_t pathLength = rootLength + moduleLength + 5;
  char* path = (char*)malloc(pathLength + 1);

  if (rootDirectory != NULL)
  {
    memcpy(path, rootDirectory, rootLength);
  }

  memcpy(path + rootLength, module, moduleLength);
  memcpy(path + rootLength + moduleLength, ".wren", 5);
  path[pathLength] = '\0';

  char* file = readFile(path);
  if (file == NULL)
  {
    free(path);
    return NULL;
  }

  return file;
}
