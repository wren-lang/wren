#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "vm.h"

#define MAX_FILE_SIZE  256 * 256

char* readFile(const char* path, size_t* length)
{
  FILE* file = fopen(path, "r");
  // TODO(bob): Handle error.

  char* buffer = malloc(MAX_FILE_SIZE);
  // TODO(bob): Hacky way to read a file!
  *length = fread(buffer, sizeof(char), MAX_FILE_SIZE, file);
  buffer[*length] = '\0';

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
