#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.
#define MAX_PATH_LENGTH 2024 // TODO: Something less arbitrary.

char rootDirectory[MAX_PATH_LENGTH];

// Reads the contents of the file at [path] and returns it as a heap allocated
// string.
//
// Returns `NULL` if the path could not be found. Exits if it was found but
// could not be read.
static char* readFile(const char* path)
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

static char* readModule(WrenVM* vm, const char* module)
{
  // The module path is relative to the root directory and with ".wren".
  size_t rootLength = strlen(rootDirectory);
  size_t moduleLength = strlen(module);
  size_t pathLength = rootLength + moduleLength + 5;
  char* path = (char*)malloc(pathLength + 1);
  memcpy(path, rootDirectory, rootLength);
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

static int runFile(WrenVM* vm, const char* path)
{
  // Use the directory where the file is as the root to resolve imports
  // relative to.
  const char* lastSlash = strrchr(path, '/');
  if (lastSlash != NULL)
  {
    memcpy(rootDirectory, path, lastSlash - path + 1);
    rootDirectory[lastSlash - path + 1] = '\0';
  }

  char* source = readFile(path);
  if (source == NULL)
  {
    fprintf(stderr, "Could not find file \"%s\".\n", path);
    exit(66);
  }

  int result;
  switch (wrenInterpret(vm, path, source))
  {
    case WREN_RESULT_SUCCESS: result = 0; break;
    case WREN_RESULT_COMPILE_ERROR: result = 65; break; // EX_DATAERR.
    case WREN_RESULT_RUNTIME_ERROR: result = 70; break; // EX_SOFTWARE.
    default:
      // Unreachable.
      result = 255;
      break;
  }
  
  wrenFreeVM(vm);
  free(source);

  return result;
}

static int runRepl(WrenVM* vm)
{
  printf("\\\\/\"-\n");
  printf(" \\_/   wren v0.0.0\n");

  // Import relative to the current directory.
  rootDirectory[0] = '\0';

  char line[MAX_LINE_LENGTH];

  for (;;)
  {
    printf("> ");

    if (fgets(line, MAX_LINE_LENGTH, stdin))
    {
      // TODO: Handle failure.
      wrenInterpret(vm, "Prompt", line);

      // TODO: Automatically print the result of expressions.
    }
    else
    {
      printf("\n");
      return 0;
    }
  }

  wrenFreeVM(vm);
  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc < 1 || argc > 2)
  {
    fprintf(stderr, "Usage: wren [file]\n");
    return 64; // EX_USAGE.
  }

  WrenConfiguration config;

  config.loadModuleFn = readModule;

  // Since we're running in a standalone process, be generous with memory.
  config.initialHeapSize = 1024 * 1024 * 100;

  // Use defaults for these.
  config.reallocateFn = NULL;
  config.minHeapSize = 0;
  config.heapGrowthPercent = 0;

  WrenVM* vm = wrenNewVM(&config);

  if (argc == 1) return runRepl(vm);
  if (argc == 2) return runFile(vm, argv[1]);
}
