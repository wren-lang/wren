#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "compiler.h"

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

  TOKEN_VAR,

  TOKEN_NAME,
  TOKEN_NUMBER,
  TOKEN_STRING,

  TOKEN_LINE,

  TOKEN_ERROR,
  TOKEN_EOF,

  MAX_TOKEN
} TokenType;

typedef struct Token_s
{
  TokenType type;
  int start;
  int end;
} Token;

typedef struct
{
  VM* vm;

  const char* source;
  size_t sourceLength;

  // The index in source of the beginning of the currently-being-lexed token.
  int tokenStart;

  // The position of the current character being lexed.
  int currentChar;

  // The most recently lexed token.
  Token current;

  // The most recently consumed/advanced token.
  Token previous;

  // The block being compiled.
  Block* block;
  int numCodes;

  // Non-zero if a compile error has occurred.
  int hasError;
} Compiler;

// Grammar:
static void statement(Compiler* compiler);
static void expression(Compiler* compiler);
static void call(Compiler* compiler);
static void primary(Compiler* compiler);
static void number(Compiler* compiler, Token* token);
static TokenType peek(Compiler* compiler);
static int match(Compiler* compiler, TokenType expected);
static void consume(Compiler* compiler, TokenType expected);
static void advance(Compiler* compiler);

// Tokens:
static void readNextToken(Compiler* compiler);
static void readName(Compiler* compiler);
static void readNumber(Compiler* compiler);
static void readString(Compiler* compiler);
static void skipWhitespace(Compiler* compiler);
static int isKeyword(Compiler* compiler, const char* keyword);
static int isName(char c);
static int isDigit(char c);
static char advanceChar(Compiler* compiler);
static char peekChar(Compiler* compiler);
static void makeToken(Compiler* compiler, TokenType type);

// Utility:
static void error(Compiler* compiler, const char* format, ...);

Block* compile(VM* vm, const char* source, size_t sourceLength)
{
  Compiler compiler;
  compiler.vm = vm;
  compiler.source = source;
  compiler.sourceLength = sourceLength;
  compiler.hasError = 0;

  compiler.tokenStart = 0;
  compiler.currentChar = 0;

  // TODO(bob): Zero-init current token.
  // Read the first token.
  advance(&compiler);

  compiler.block = malloc(sizeof(Block));
  // TODO(bob): Hack! make variable sized.
  compiler.block->bytecode = malloc(sizeof(Code) * 1024);

  // TODO(bob): Hack! make variable sized.
  compiler.block->constants = malloc(sizeof(Value) * 256);
  compiler.block->numConstants = 0;

  compiler.numCodes = 0;

  do
  {
    statement(&compiler);
    // TODO(bob): Discard previous value.
  } while (!match(&compiler, TOKEN_EOF));

  compiler.block->bytecode[compiler.numCodes++] = CODE_END;

  return compiler.hasError ? NULL : compiler.block;
}

void statement(Compiler* compiler)
{
  /*
  if (match(compiler, TOKEN_VAR))
  {
    Token* name = consume(compiler, TOKEN_NAME);
    Node* initializer = NULL;
    if (match(compiler, TOKEN_EQ))
    {
      initializer = expression(parser);
    }
    if (peek(parser) != TOKEN_OUTDENT) consume(compiler, TOKEN_LINE);

    NodeVar* node = malloc(sizeof(NodeVar));
    node->node.type = NODE_VAR;
    node->name = name;
    node->initializer = initializer;
    return (Node*)node;
  }
  */

  // Statement expression.
  expression(compiler);
  consume(compiler, TOKEN_LINE);
}

void expression(Compiler* compiler)
{
  call(compiler);
}

// Method calls like:
//
// foo.bar
// foo.bar(arg, arg)
// foo.bar { block } other { block }
void call(Compiler* compiler)
{
  primary(compiler);

  if (match(compiler, TOKEN_DOT))
  {
    consume(compiler, TOKEN_NAME);
    int symbol = getSymbol(compiler->vm,
                           compiler->source + compiler->previous.start,
                           compiler->previous.end - compiler->previous.start);
    printf("symbol %d\n", symbol);


    // Compile the method call.
    compiler->block->bytecode[compiler->numCodes++] = CODE_CALL;
    compiler->block->bytecode[compiler->numCodes++] = symbol;
  }
}

void primary(Compiler* compiler)
{
  if (match(compiler, TOKEN_NUMBER))
  {
    number(compiler, &compiler->previous);
  }
}

void number(Compiler* compiler, Token* token)
{
  char* end;
  long value = strtol(compiler->source + token->start, &end, 10);
  // TODO(bob): Check errno == ERANGE here.
  if (end == compiler->source + token->start)
  {
    error(compiler, "Invalid number literal.");
    value = 0;
  }

  // Define a constant for the literal.
  // TODO(bob): See if constant with same value already exists.
  Value constant = malloc(sizeof(Obj));
  constant->type = OBJ_INT;
  constant->flags = 0;

  // TODO(bob): Handle truncation!
  constant->value = (int)value;

  compiler->block->constants[compiler->block->numConstants++] = constant;

  // Compile the code to load the constant.
  compiler->block->bytecode[compiler->numCodes++] = CODE_CONSTANT;
  compiler->block->bytecode[compiler->numCodes++] = compiler->block->numConstants - 1;
}

TokenType peek(Compiler* compiler)
{
  return compiler->current.type;
}

// TODO(bob): Make a bool type?
int match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return 0;

  advance(compiler);
  return 1;
}

void consume(Compiler* compiler, TokenType expected)
{
  advance(compiler);
  if (compiler->previous.type != expected)
  {
    // TODO(bob): Better error.
    error(compiler, "Expected %d, got %d.\n", expected, compiler->previous.type);
  }
}

void advance(Compiler* compiler)
{
  // TODO(bob): Check for EOF.
  compiler->previous = compiler->current;
  readNextToken(compiler);
}

void readNextToken(Compiler* compiler)
{
  while (peekChar(compiler) != '\0')
  {
    compiler->tokenStart = compiler->currentChar;

    char c = advanceChar(compiler);
    switch (c)
    {
      case '(': makeToken(compiler, TOKEN_LEFT_PAREN); return;
      case ')': makeToken(compiler, TOKEN_RIGHT_PAREN); return;
      case '[': makeToken(compiler, TOKEN_LEFT_BRACKET); return;
      case ']': makeToken(compiler, TOKEN_RIGHT_BRACKET); return;
      case '{': makeToken(compiler, TOKEN_LEFT_BRACE); return;
      case '}': makeToken(compiler, TOKEN_RIGHT_BRACE); return;
      case ':': makeToken(compiler, TOKEN_COLON); return;
      case '.': makeToken(compiler, TOKEN_DOT); return;
      case ',': makeToken(compiler, TOKEN_COMMA); return;
      case '*': makeToken(compiler, TOKEN_STAR); return;
      case '/': makeToken(compiler, TOKEN_SLASH); return;
      case '%': makeToken(compiler, TOKEN_PERCENT); return;
      case '+': makeToken(compiler, TOKEN_PLUS); return;
      case '-': makeToken(compiler, TOKEN_MINUS); return;
      case '|': makeToken(compiler, TOKEN_PIPE); return;
      case '&': makeToken(compiler, TOKEN_AMP); return;
      case '=':
        if (peekChar(compiler) == '=')
        {
          advanceChar(compiler);
          makeToken(compiler, TOKEN_EQEQ);
        }
        else
        {
          makeToken(compiler, TOKEN_EQ);
        }
        return;

      case '<':
        if (peekChar(compiler) == '=')
        {
          advanceChar(compiler);
          makeToken(compiler, TOKEN_LTEQ);
        }
        else
        {
          makeToken(compiler, TOKEN_LT);
        }
        return;

      case '>':
        if (peekChar(compiler) == '=')
        {
          advanceChar(compiler);
          makeToken(compiler, TOKEN_GTEQ);
        }
        else
        {
          makeToken(compiler, TOKEN_GT);
        }
        return;

      case '!':
        if (peekChar(compiler) == '=')
        {
          advanceChar(compiler);
          makeToken(compiler, TOKEN_BANGEQ);
        }
        else
        {
          makeToken(compiler, TOKEN_BANG);
        }
        return;

      case '\n': makeToken(compiler, TOKEN_LINE); return;

      case ' ': skipWhitespace(compiler); break;
      case '"': readString(compiler); return;

      default:
        if (isName(c))
        {
          readName(compiler);
        }
        else if (isDigit(c))
        {
          readNumber(compiler);
        }
        else
        {
          makeToken(compiler, TOKEN_ERROR);
        }
        return;
    }
  }

  // If we get here, we're out of source, so just make EOF tokens.
  compiler->tokenStart = compiler->currentChar;
  makeToken(compiler, TOKEN_EOF);
}

void readName(Compiler* compiler)
{
  // TODO(bob): Handle digits and EOF.
  while (isName(peekChar(compiler)) || isDigit(peekChar(compiler))) advanceChar(compiler);

  TokenType type = TOKEN_NAME;

  if (isKeyword(compiler, "var")) type = TOKEN_VAR;

  makeToken(compiler, type);
}

int isKeyword(Compiler* compiler, const char* keyword)
{
  size_t length = compiler->currentChar - compiler->tokenStart;
  size_t keywordLength = strlen(keyword);
  return length == keywordLength &&
  strncmp(compiler->source + compiler->tokenStart, keyword, length) == 0;
}

void readNumber(Compiler* compiler)
{
  // TODO(bob): Floating point, hex, scientific, etc.
  while (isDigit(peekChar(compiler))) advanceChar(compiler);

  makeToken(compiler, TOKEN_NUMBER);
}

void readString(Compiler* compiler)
{
  // TODO(bob): Escape sequences, EOL, EOF, etc.
  while (advanceChar(compiler) != '"');

  makeToken(compiler, TOKEN_STRING);
}

void skipWhitespace(Compiler* compiler)
{
  while (peekChar(compiler) == ' ') advanceChar(compiler);
}

int isName(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int isDigit(char c)
{
  return c >= '0' && c <= '9';
}

char advanceChar(Compiler* compiler)
{
  char c = peekChar(compiler);
  compiler->currentChar++;
  return c;
}

char peekChar(Compiler* compiler)
{
  return compiler->source[compiler->currentChar];
}

void makeToken(Compiler* compiler, TokenType type)
{
  compiler->current.type = type;
  compiler->current.start = compiler->tokenStart;
  compiler->current.end = compiler->currentChar;
}

void error(Compiler* compiler, const char* format, ...)
{
  compiler->hasError = 1;
  printf("Compile error on '");

  for (int i = compiler->previous.start; i < compiler->previous.end; i++)
  {
    putchar(compiler->source[i]);
  }

  printf("': ");

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}
