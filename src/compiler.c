#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "compiler.h"

// Note: if you add new token types, make sure to update the arrays below.
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

typedef void (*CompileFn)(Compiler*, Token*);

typedef struct
{
  CompileFn fn;
  int precedence;
} InfixCompiler;

// Parsing:

/*
static void block(Compiler* compiler);
*/
static void statementLike(Compiler* compiler);
static void expression(Compiler* compiler);
static void compilePrecedence(Compiler* compiler, int precedence);
static void prefixLiteral(Compiler* compiler, Token* token);
static void infixCall(Compiler* compiler, Token* token);
static void infixBinaryOp(Compiler* compiler, Token* token);
static TokenType peek(Compiler* compiler);
static int match(Compiler* compiler, TokenType expected);
static void consume(Compiler* compiler, TokenType expected);
static void advance(Compiler* compiler);

// Lexing:
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

enum
{
  PREC_NONE,
  PREC_LOWEST,

  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_BITWISE,    // | &
  PREC_TERM,       // + -
  PREC_FACTOR,     // * / %
  PREC_CALL        // ()
};

CompileFn prefixCompilers[] = {
  NULL, // TOKEN_LEFT_PAREN
  NULL, // TOKEN_RIGHT_PAREN
  NULL, // TOKEN_LEFT_BRACKET
  NULL, // TOKEN_RIGHT_BRACKET
  NULL, // TOKEN_LEFT_BRACE
  NULL, // TOKEN_RIGHT_BRACE
  NULL, // TOKEN_COLON
  NULL, // TOKEN_DOT
  NULL, // TOKEN_COMMA
  NULL, // TOKEN_STAR
  NULL, // TOKEN_SLASH
  NULL, // TOKEN_PERCENT
  NULL, // TOKEN_PLUS
  NULL, // TOKEN_MINUS
  NULL, // TOKEN_PIPE
  NULL, // TOKEN_AMP
  NULL, // TOKEN_BANG
  NULL, // TOKEN_EQ
  NULL, // TOKEN_LT
  NULL, // TOKEN_GT
  NULL, // TOKEN_LTEQ
  NULL, // TOKEN_GTEQ
  NULL, // TOKEN_EQEQ
  NULL, // TOKEN_BANGEQ
  NULL, // TOKEN_ELSE
  NULL, // TOKEN_IF
  NULL, // TOKEN_VAR
  prefixLiteral, // TOKEN_NAME
  prefixLiteral, // TOKEN_NUMBER
  prefixLiteral, // TOKEN_STRING
  NULL, // TOKEN_LINE
  NULL, // TOKEN_ERROR
  NULL // TOKEN_EOF
};

// The indices in this array correspond to TOKEN enum values.
InfixCompiler infixCompilers[] = {
  { infixCall, PREC_CALL }, // TOKEN_LEFT_PAREN
  { NULL, PREC_NONE }, // TOKEN_RIGHT_PAREN
  { NULL, PREC_NONE }, // TOKEN_LEFT_BRACKET
  { NULL, PREC_NONE }, // TOKEN_RIGHT_BRACKET
  { NULL, PREC_NONE }, // TOKEN_LEFT_BRACE
  { NULL, PREC_NONE }, // TOKEN_RIGHT_BRACE
  { NULL, PREC_NONE }, // TOKEN_COLON
  { NULL, PREC_NONE }, // TOKEN_DOT
  { NULL, PREC_NONE }, // TOKEN_COMMA
  { infixBinaryOp, PREC_FACTOR }, // TOKEN_STAR
  { infixBinaryOp, PREC_FACTOR }, // TOKEN_SLASH
  { infixBinaryOp, PREC_FACTOR }, // TOKEN_PERCENT
  { infixBinaryOp, PREC_TERM }, // TOKEN_PLUS
  { infixBinaryOp, PREC_TERM }, // TOKEN_MINUS
  { infixBinaryOp, PREC_BITWISE }, // TOKEN_PIPE
  { infixBinaryOp, PREC_BITWISE }, // TOKEN_AMP
  { NULL, PREC_NONE }, // TOKEN_BANG
  { NULL, PREC_NONE }, // TOKEN_EQ
  { infixBinaryOp, PREC_COMPARISON }, // TOKEN_LT
  { infixBinaryOp, PREC_COMPARISON }, // TOKEN_GT
  { infixBinaryOp, PREC_COMPARISON }, // TOKEN_LTEQ
  { infixBinaryOp, PREC_COMPARISON }, // TOKEN_GTEQ
  { infixBinaryOp, PREC_EQUALITY }, // TOKEN_EQEQ
  { infixBinaryOp, PREC_EQUALITY }, // TOKEN_BANGEQ
  { NULL, PREC_NONE }, // TOKEN_ELSE
  { NULL, PREC_NONE }, // TOKEN_IF
  { NULL, PREC_NONE }, // TOKEN_VAR
  { NULL, PREC_NONE }, // TOKEN_NAME
  { NULL, PREC_NONE }, // TOKEN_NUMBER
  { NULL, PREC_NONE }, // TOKEN_STRING
  { NULL, PREC_NONE }, // TOKEN_LINE
  { NULL, PREC_NONE }, // TOKEN_ERROR
  { NULL, PREC_NONE } // TOKEN_EOF
};

Block* compile(const char* source, size_t sourceLength)
{
  Compiler compiler;
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

  // TODO(bob): Copied from block(). Unify.
  do
  {
    statementLike(&compiler);
  } while (!match(&compiler, TOKEN_EOF));

  compiler.block->bytecode[compiler.numCodes++] = CODE_END;

  return compiler.hasError ? NULL : compiler.block;
}

/*
void block(Compiler* compiler)
{
  consume(compiler, TOKEN_INDENT);

  NodeSequence* sequence = malloc(sizeof(NodeSequence));
  sequence->node.type = NODE_SEQUENCE;
  sequence->nodes = NULL;

  NodeList** nodes = &sequence->nodes;
  do
  {
    Node* node = statementLike(compiler);
    *nodes = malloc(sizeof(NodeList));
    (*nodes)->node = node;
    (*nodes)->next = NULL;
    nodes = &(*nodes)->next;

  } while (!match(compiler, TOKEN_OUTDENT));

  return (Node*)sequence;
}
*/

void statementLike(Compiler* compiler)
{
  /*
  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    expression(compiler);

    consume(compiler, TOKEN_COLON);

    // Compile the then arm.
    block(compiler);

    // Compile the else arm.
    if (match(compiler, TOKEN_ELSE))
    {
      consume(compiler, TOKEN_COLON);
      block(parser);
    }

    return;
  }

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
  compilePrecedence(compiler, PREC_LOWEST);
}

void compilePrecedence(Compiler* compiler, int precedence)
{
  advance(compiler);
  CompileFn prefix = prefixCompilers[compiler->previous.type];

  if (prefix == NULL)
  {
    // TODO(bob): Handle error better.
    error(compiler, "No prefix parser.");
    exit(1);
  }

  prefix(compiler, &compiler->previous);

  while (precedence <= infixCompilers[compiler->current.type].precedence)
  {
    advance(compiler);
    CompileFn infix = infixCompilers[compiler->previous.type].fn;
    infix(compiler, &compiler->previous);
  }
}

void prefixLiteral(Compiler* compiler, Token* token)
{
  // TODO(bob): Get actual value from token!
  // Define a constant for the literal.
  // TODO(bob): See if constant with same value already exists.
  Value constant = malloc(sizeof(Obj));
  constant->type = OBJ_INT;
  constant->flags = 0;
  constant->value = 234;

  compiler->block->constants[compiler->block->numConstants++] = constant;

  // Compile the code to load the constant.
  compiler->block->bytecode[compiler->numCodes++] = CODE_CONSTANT;
  compiler->block->bytecode[compiler->numCodes++] = compiler->block->numConstants - 1;
}

void infixCall(Compiler* compiler, Token* token)
{
  printf("infix calls not implemented\n");
  exit(1);
  /*
  NodeList* args = NULL;
  if (match(compiler, TOKEN_RIGHT_PAREN) == NULL)
  {
    NodeList** arg = &args;
    do
    {
      *arg = malloc(sizeof(NodeList));
      (*arg)->node = expression(parser);
      (*arg)->next = NULL;
      arg = &(*arg)->next;
    }
    while (match(compiler, TOKEN_COMMA) != NULL);

    consume(compiler, TOKEN_RIGHT_PAREN);
  }

  NodeCall* node = malloc(sizeof(NodeCall));
  node->node.type = NODE_CALL;
  node->fn = left;
  node->args = args;

  return (Node*)node;
  */
}

void infixBinaryOp(Compiler* compiler, Token* token)
{
  printf("infix binary ops not implemented\n");
  exit(1);
  /*
  // TODO(bob): Support right-associative infix. Needs to do precedence
  // - 1 here to be right-assoc.
  Node* right = parsePrecedence(parser,
                                infixParsers[token->type].precedence);

  NodeBinaryOp* node = malloc(sizeof(NodeBinaryOp));
  node->node.type = NODE_BINARY_OP;
  node->left = left;
  node->op = token;
  node->right = right;

  return (Node*)node;
  */
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

  if (isKeyword(compiler, "else")) type = TOKEN_ELSE;
  else if (isKeyword(compiler, "if")) type = TOKEN_IF;
  else if (isKeyword(compiler, "var")) type = TOKEN_VAR;

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

  for (int i = compiler->current.start; i < compiler->current.end; i++)
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
