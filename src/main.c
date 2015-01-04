#define _GNU_SOURCE // Makes getline() available in GCC.

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren.h"

// This is the source file for the standalone command line interpreter. It is
// not needed if you are embedding Wren in an application.

static void failIf(bool condition, int exitCode, const char* format, ...)
{
  if (!condition) return;

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(exitCode);
}

static char* readFile(const char* path)
{
  FILE* file = fopen(path, "r");
  failIf(file == NULL, 66, "Could not open file \"%s\".\n", path);

  // Find out how big the file is.
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // Allocate a buffer for it.
  char* buffer = malloc(fileSize + 1);
  failIf(buffer == NULL, 74, "Could not read file \"%s\".\n", path);

  // Read the entire file.
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  failIf(bytesRead < fileSize, 74, "Could not read file \"%s\".\n", path);

  // Terminate the string.
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

static int runFile(WrenVM* vm, const char* path)
{
  char* source = readFile(path);

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

  for (;;)
  {
    printf("> ");

    char* line = NULL;
    size_t size = 0;
    ssize_t numRead = getline(&line, &size, stdin);

    // If stdin was closed (usually meaning the user entered Ctrl-D), exit.
    if (numRead == -1 || feof(stdin))
    {
      printf("\n");
      return 0;
    }

    // TODO: Handle failure.
    wrenInterpret(vm, "Prompt", line);

    free(line);

    // TODO: Figure out how this should work with wren API.
    /*
    ObjFn* fn = compile(vm, line);

    if (fn != NULL)
    {
      Value result = interpret(vm, fn);
      printf("= ");
      printValue(result);
      printf("\n");
    }
    */
  }

  wrenFreeVM(vm);
  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc < 1 || argc > 2)
  {
    fprintf(stderr, "Usage: wren [file]");
    return 64; // EX_USAGE.
  }

  WrenConfiguration config = {
    .reallocateFn = NULL,
    .heapGrowthPercent = 0,
    .minHeapSize = 0,
    // Since we're running in a standalone process, be generous with memory.
    .initialHeapSize = 1024 * 1024 * 100
  };

  WrenVM* vm = wrenNewVM(&config);

  if (argc == 1) return runRepl(vm);
  if (argc == 2) return runFile(vm, argv[1]);
}
