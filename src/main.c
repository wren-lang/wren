#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren.h"

// This is the source file for the standalone command line interpreter. It is
// not needed if you are embedding Wren in an application.

// TODO(bob): Don't hardcode this.
#define MAX_LINE 1024

static void failIf(int condition, int exitCode, const char* format, ...)
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

static void* reallocate(void* memory, size_t oldSize, size_t newSize)
{
  return realloc(memory, newSize);
}

static int runFile(const char* path)
{
  char* source = readFile(path);
  WrenVM* vm = wrenNewVM(reallocate);

  int result = wrenInterpret(vm, source);

  wrenFreeVM(vm);
  free(source);

  return result;
}

static int runRepl()
{
  WrenVM* vm = wrenNewVM(reallocate);

  for (;;)
  {
    printf("> ");
    char line[MAX_LINE];
    fgets(line, MAX_LINE, stdin);

    // TODO(bob): Handle failure.
    wrenInterpret(vm, line);
    // TODO(bob): Figure out how this should work with wren API.
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
  if (argc == 1) return runRepl();
  if (argc == 2) return runFile(argv[1]);

  fprintf(stderr, "Usage: wren [file]");
  return 1;
}
