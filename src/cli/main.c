#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "wren.h"

#define MAX_LINE_LENGTH 1024 // TODO: Something less arbitrary.

static int runFile(WrenVM* vm, const char* path)
{
  // Use the directory where the file is as the root to resolve imports
  // relative to.
  const char* lastSlash = strrchr(path, '/');
  if (lastSlash != NULL)
  {
    char* root = (char*)malloc(lastSlash - path + 2);
    memcpy(root, path, lastSlash - path + 1);
    root[lastSlash - path + 1] = '\0';
    setRootDirectory(root);
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
