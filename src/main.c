#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "vm.h"

void failIf(int condition, int exitCode, const char* format, ...)
{
  if (!condition) return;

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(exitCode);
}

char* readFile(const char* path, size_t* length)
{
  FILE* file = fopen(path, "r");
  failIf(file == NULL, 66, "Could not open file \"%s\".\n", path);

  // Find out how big the file is.
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // Allocate a buffer for it.
  char* buffer = malloc(fileSize);
  failIf(buffer == NULL, 74, "Could not read file \"%s\".\n", path);

  // Read the entire file.
  *length = fread(buffer, sizeof(char), fileSize, file);
  failIf(*length < fileSize, 74, "Could not read file \"%s\".\n", path);

  fclose(file);
  return buffer;
}

int main(int argc, const char * argv[])
{
  // TODO(bob): Validate command line arguments.
  size_t length;
  char* source = readFile(argv[1], &length);
  VM* vm = newVM();
  ObjBlock* block = compile(vm, source, length);

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
