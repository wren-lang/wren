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

char* wrenFilePath(const char* name)
{
  // The module path is relative to the root directory and with ".wren".
  size_t rootLength = rootDirectory == NULL ? 0 : strlen(rootDirectory);
  size_t nameLength = strlen(name);
  size_t pathLength = rootLength + nameLength + 5;
  char* path = (char*)malloc(pathLength + 1);

  if (rootDirectory != NULL)
  {
    memcpy(path, rootDirectory, rootLength);
  }

  memcpy(path + rootLength, name, nameLength);
  memcpy(path + rootLength + nameLength, ".wren", 5);
  path[pathLength] = '\0';

  return path;
}

char* readModule(WrenVM* vm, const char* module)
{
  // First try to load the module with a ".wren" extension.
  char* modulePath = wrenFilePath(module);
  char* moduleContents = readFile(modulePath);

  // If no contents could be loaded treat the module name as specifying a
  // package and try to load the "module.wren" file in the package.
  if (moduleContents == NULL)
  {
    free(modulePath);

    size_t moduleLength = strlen(module);
    size_t modulePackageLength = moduleLength + 7;
    char* packageModule = (char*)malloc(modulePackageLength + 1);
    memcpy(packageModule, module, moduleLength);
    memcpy(packageModule + moduleLength, "/module", 7);
    packageModule[modulePackageLength] = '\0';

    char* packageModulePath = wrenFilePath(packageModule);
    moduleContents = readFile(packageModulePath);

    if (moduleContents == NULL)
    {
      free(packageModulePath);
      return NULL;
    }
  }

  return moduleContents;
}
