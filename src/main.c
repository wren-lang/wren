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

  if (block)
  {
    interpret(vm, block);
  }

  freeVM(vm);
  free(source);

  return 0;
}

/*
static void dumpTokens(Buffer* buffer, Token* token)
{
  while (token)
  {
    switch (token->type)
    {
      case TOKEN_LINE: printf("(line)"); break;
      case TOKEN_ERROR: printf("(error)"); break;
      case TOKEN_EOF: printf("(eof)"); break;
      default:
        printf("⊏");
        for (int i = token->start; i < token->end; i++)
        {
          putchar(buffer->bytes[i]);
        }
        printf("⊐");
    }
    token = token->next;
  }
  printf("\n");
}
*/
