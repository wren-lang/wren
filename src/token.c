#include <stdio.h>
#include <string.h>

#include "token.h"

Buffer* newBuffer(size_t size)
{
  Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
  buffer->bytes = (char*)malloc(size);
  buffer->size = size;

  return buffer;
}

void freeBuffer(Buffer* buffer)
{
  free(buffer->bytes);
  free(buffer);
}

Token* newToken(TokenType type, int start, int end)
{
  Token* token = (Token*)malloc(sizeof(Token));
  token->type = type;
  token->start = start;
  token->end = end;
  token->prev = NULL;
  token->next = NULL;
  return token;
}

void printToken(Buffer* buffer, Token* token)
{
  for (int i = token->start; i < token->end; i++)
  {
    putchar(buffer->bytes[i]);
  }
}

Token* unlinkToken(Token* token)
{
  if (token->next) token->next->prev = token->prev;
  if (token->prev) token->prev->next = token->next;

  token->next = NULL;
  token->prev = NULL;

  return token;
}
