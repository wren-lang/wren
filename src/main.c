#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "vm.h"

// TODO(bob): Don't hardcode this.
#define MAX_LINE 1024

void failIf(int condition, int exitCode, const char* format, ...)
{
  if (!condition) return;

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(exitCode);
}

char* readFile(const char* path)
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

int runFile(const char* path)
{
  char* source = readFile(path);
  VM* vm = newVM();
  ObjBlock* block = compile(vm, source);

  int exitCode = 0;
  if (block)
  {
    interpret(vm, block);
  }
  else
  {
    exitCode = 1;
  }

  freeVM(vm);
  free(source);

  return exitCode;
}

int runRepl()
{
  VM* vm = newVM();

  for (;;)
  {
    printf("> ");
    char line[MAX_LINE];
    fgets(line, MAX_LINE, stdin);
    // TODO(bob): Handle failure.
    ObjBlock* block = compile(vm, line);

    if (block != NULL)
    {
      Value result = interpret(vm, block);
      printf("= ");
      printValue(result);
      printf("\n");
    }
  }

  freeVM(vm);
  return 0;
}

int main(int argc, const char* argv[])
{
  if (argc == 1) return runRepl();
  if (argc == 2) return runFile(argv[1]);

  fprintf(stderr, "Usage: wren [file]");
  return 1;
}
