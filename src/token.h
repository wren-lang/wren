#ifndef wren_token_h
#define wren_token_h

#include <stdlib.h>

// TODO(bob): Move somewhere else?
typedef struct Buffer_s
{
  char*  bytes;
  size_t size;
} Buffer;

// Note: if you add new token types, make sure to update the parser arrays in
// parser.c.
typedef enum
{
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACKET,
  TOKEN_RIGHT_BRACKET,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COLON,
  TOKEN_DOT,
  TOKEN_COMMA,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_PIPE,
  TOKEN_AMP,
  TOKEN_BANG,
  TOKEN_EQ,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_LTEQ,
  TOKEN_GTEQ,
  TOKEN_EQEQ,
  TOKEN_BANGEQ,

  TOKEN_ELSE,
  TOKEN_IF,
  TOKEN_VAR,

  TOKEN_EMBEDDED,
  TOKEN_NAME,
  TOKEN_NUMBER,
  TOKEN_STRING,

  TOKEN_LINE,
  TOKEN_WHITESPACE,

  TOKEN_ERROR,
  TOKEN_EOF,

  MAX_TOKEN
} TokenType;

typedef struct Token_s
{
  TokenType type;
  int start;
  int end;

  struct Token_s* prev;
  struct Token_s* next;
} Token;

Buffer* newBuffer(size_t size);
void freeBuffer(Buffer* buffer);

// Creates a new unlinked token.
Token* newToken(TokenType type, int start, int end);

// Prints the verbatim source text of the token.
void printToken(Buffer* buffer, Token* token);

// Removes the token from the list containing it. Does not free it.
Token* unlinkToken(Token* token);

#endif
