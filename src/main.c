#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

#define MAX_FILE_SIZE  256 * 256

static void dumpTokens(Buffer* buffer, Token* token);

Buffer* readFile(const char* path)
{
  FILE* file = fopen(path, "r");
  // TODO(bob): Handle error.

  Buffer* buffer = newBuffer(MAX_FILE_SIZE);
  // TODO(bob): Hacky way to read a file!
  size_t read = fread(buffer->bytes, sizeof(char), MAX_FILE_SIZE, file);
  buffer->bytes[read] = '\0';

  fclose(file);
  return buffer;
}

int main(int argc, const char * argv[])
{
  // TODO(bob): Validate command line arguments.
  Buffer* buffer = readFile(argv[1]);
  Token* tokens = tokenize(buffer);
  //printf("Raw tokens:\n");
  //dumpTokens(buffer, tokens);
  //printf("Cleaned tokens:\n");
  dumpTokens(buffer, tokens);

  /*Node* node =*/ parse(buffer, tokens);

  /*
  if (defns) generate(buffer, defns);

  // TODO(bob): Free tokens.
  // TODO(bob): Free ast.
   */

  freeBuffer(buffer);

  return 0;
}

static void dumpTokens(Buffer* buffer, Token* token)
{
  while (token)
  {
      switch (token->type)
      {
          case TOKEN_INDENT: printf("(in)"); break;
          case TOKEN_OUTDENT: printf("(out)"); break;
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