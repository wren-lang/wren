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

  TOKEN_CLASS,
  TOKEN_META,
  TOKEN_VAR,

  TOKEN_NAME,
  TOKEN_NUMBER,
  TOKEN_STRING,

  TOKEN_LINE,

  TOKEN_ERROR,
  TOKEN_EOF
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

  // Non-zero if subsequent newline tokens should be discarded.
  int skipNewlines;

  // Non-zero if a syntax or compile error has occurred.
  int hasError;
} Parser;

typedef struct sCompiler
{
  Parser* parser;

  // The compiler for the block enclosing this one, or NULL if it's the
  // top level.
  struct sCompiler* parent;

  // The block being compiled.
  ObjBlock* block;
  int numCodes;

  // Symbol table for declared local variables in this block.
  SymbolTable locals;
} Compiler;

static ObjBlock* compileBlock(Parser* parser, Compiler* parent,
                              TokenType endToken);
static int addConstant(Compiler* compiler, Value constant);

// Grammar:
static void statement(Compiler* compiler);
static void expression(Compiler* compiler);
static void call(Compiler* compiler);
static void primary(Compiler* compiler);
static void number(Compiler* compiler);
static TokenType peek(Compiler* compiler);
static int match(Compiler* compiler, TokenType expected);
static void consume(Compiler* compiler, TokenType expected);
static void advance(Parser* parser);

// Tokens:

// Lex the next token in the source file and store it in parser.current. Omits
// newlines that aren't meaningful.
static void readNextToken(Parser* parser);

// Lex the next token and store it in parser.current. Does not do any newline
// filtering.
static void readRawToken(Parser* parser);

static void readName(Parser* parser);
static void readNumber(Parser* parser);
static void readString(Parser* parser);
static void skipLineComment(Parser* parser);
static void skipWhitespace(Parser* parser);
static int isKeyword(Parser* parser, const char* keyword);
static int isName(char c);
static int isDigit(char c);
static char advanceChar(Parser* parser);
static char peekChar(Parser* parser);
static void makeToken(Parser* parser, TokenType type);

// Utility:
static void initCompiler(Compiler* compiler, Parser* parser, Compiler* parent);
static void emit(Compiler* compiler, Code code);
static void error(Compiler* compiler, const char* format, ...);

ObjBlock* compile(VM* vm, const char* source, size_t sourceLength)
{
  Parser parser;
  parser.vm = vm;
  parser.source = source;
  parser.sourceLength = sourceLength;
  parser.hasError = 0;

  // Ignore leading newlines.
  parser.skipNewlines = 1;

  parser.tokenStart = 0;
  parser.currentChar = 0;

  // Zero-init the current token. This will get copied to previous when
  // advance() is called below.
  parser.current.type = TOKEN_EOF;
  parser.current.start = 0;
  parser.current.end = 0;

  // Read the first token.
  advance(&parser);

  return compileBlock(&parser, NULL, TOKEN_EOF);
}

ObjBlock* compileBlock(Parser* parser, Compiler* parent, TokenType endToken)
{
  Compiler compiler;
  initCompiler(&compiler, parser, parent);

  for (;;)
  {
    statement(&compiler);

    consume(&compiler, TOKEN_LINE);

    if (match(&compiler, endToken)) break;

    // Discard the result of the previous expression.
    emit(&compiler, CODE_POP);
  }

  emit(&compiler, CODE_END);

  compiler.block->numLocals = compiler.locals.count;

  return parser->hasError ? NULL : compiler.block;
}

int addConstant(Compiler* compiler, Value constant)
{
  compiler->block->constants[compiler->block->numConstants++] = constant;
  return compiler->block->numConstants - 1;
}

void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_CLASS))
  {
    consume(compiler, TOKEN_NAME);

    // TODO(bob): Copied from below. Unify.
    int local = addSymbol(&compiler->locals,
        compiler->parser->source + compiler->parser->previous.start,
        compiler->parser->previous.end - compiler->parser->previous.start);

    if (local == -1)
    {
      error(compiler, "Local variable is already defined.");
    }

    // Create the empty class.
    emit(compiler, CODE_CLASS);

    // Store it in its name.
    emit(compiler, CODE_STORE_LOCAL);
    emit(compiler, local);

    // Compile the method definitions.
    consume(compiler, TOKEN_LEFT_BRACE);

    while (!match(compiler, TOKEN_RIGHT_BRACE))
    {
      // Method name.
      consume(compiler, TOKEN_NAME);
      int symbol = internSymbol(compiler);

      consume(compiler, TOKEN_LEFT_BRACE);
      ObjBlock* method = compileBlock(compiler->parser, compiler,
                                      TOKEN_RIGHT_BRACE);
      consume(compiler, TOKEN_LINE);

      // Add the block to the constant table.
      int constant = addConstant(compiler, (Value)method);

      // Compile the code to define the method it.
      emit(compiler, CODE_METHOD);
      emit(compiler, symbol);
      emit(compiler, constant);
    }

    return;
  }

  if (match(compiler, TOKEN_VAR))
  {
    consume(compiler, TOKEN_NAME);
    int local = addSymbol(&compiler->locals,
        compiler->parser->source + compiler->parser->previous.start,
        compiler->parser->previous.end - compiler->parser->previous.start);

    if (local == -1)
    {
      error(compiler, "Local variable is already defined.");
    }

    // TODO(bob): Allow uninitialized vars?
    consume(compiler, TOKEN_EQ);

    // Compile the initializer.
    expression(compiler);

    emit(compiler, CODE_STORE_LOCAL);
    emit(compiler, local);
    return;
  }

  // Statement expression.
  expression(compiler);
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
    int symbol = internSymbol(compiler);

    // Compile the method call.
    emit(compiler, CODE_CALL);
    emit(compiler, symbol);
  }
}

void primary(Compiler* compiler)
{
  // Block.
  if (match(compiler, TOKEN_LEFT_BRACE))
  {
    ObjBlock* block = compileBlock(
        compiler->parser, compiler, TOKEN_RIGHT_BRACE);

    // Add the block to the constant table.
    compiler->block->constants[compiler->block->numConstants++] = (Value)block;

    // Compile the code to load it.
    emit(compiler, CODE_CONSTANT);
    emit(compiler, compiler->block->numConstants - 1);
    return;
  }

  // Variable name.
  if (match(compiler, TOKEN_NAME))
  {
    int local = findSymbol(&compiler->locals,
        compiler->parser->source + compiler->parser->previous.start,
        compiler->parser->previous.end - compiler->parser->previous.start);
    if (local == -1)
    {
      // TODO(bob): Look for globals or names in outer scopes.
      error(compiler, "Unknown variable.");
    }

    emit(compiler, CODE_LOAD_LOCAL);
    emit(compiler, local);
    return;
  }

  // Number.
  if (match(compiler, TOKEN_NUMBER))
  {
    number(compiler);
    return;
  }
}

void number(Compiler* compiler)
{
  Token* token = &compiler->parser->previous;
  char* end;
  // TODO(bob): Parse actual double!
  long value = strtol(compiler->parser->source + token->start, &end, 10);
  // TODO(bob): Check errno == ERANGE here.
  if (end == compiler->parser->source + token->start)
  {
    error(compiler, "Invalid number literal.");
    value = 0;
  }

  // Define a constant for the literal.
  int constant = addConstant(compiler, (Value)makeNum((double)value));

  // Compile the code to load the constant.
  emit(compiler, CODE_CONSTANT);
  emit(compiler, constant);
}

TokenType peek(Compiler* compiler)
{
  return compiler->parser->current.type;
}

// TODO(bob): Make a bool type?
int match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return 0;

  advance(compiler->parser);
  return 1;
}

void consume(Compiler* compiler, TokenType expected)
{
  advance(compiler->parser);
  if (compiler->parser->previous.type != expected)
  {
    // TODO(bob): Better error.
    error(compiler, "Expected %d, got %d.\n", expected,
          compiler->parser->previous.type);
  }
}

void advance(Parser* parser)
{
  // TODO(bob): Check for EOF.
  parser->previous = parser->current;
  readNextToken(parser);
}

void readNextToken(Parser* parser)
{
  for (;;)
  {
    readRawToken(parser);
    switch (parser->current.type)
    {
      case TOKEN_LINE:
        if (!parser->skipNewlines)
        {
          // Collapse multiple newlines into one.
          parser->skipNewlines = 1;

          // Emit this newline.
          return;
        }
        break;

        // Discard newlines after tokens that cannot end an expression.
      case TOKEN_LEFT_PAREN:
      case TOKEN_LEFT_BRACKET:
      case TOKEN_LEFT_BRACE:
      case TOKEN_DOT:
      case TOKEN_COMMA:
      case TOKEN_STAR:
      case TOKEN_SLASH:
      case TOKEN_PERCENT:
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_PIPE:
      case TOKEN_AMP:
      case TOKEN_BANG:
      case TOKEN_EQ:
      case TOKEN_LT:
      case TOKEN_GT:
      case TOKEN_LTEQ:
      case TOKEN_GTEQ:
      case TOKEN_EQEQ:
      case TOKEN_BANGEQ:
      case TOKEN_CLASS:
      case TOKEN_META:
      case TOKEN_VAR:
        parser->skipNewlines = 1;

        // Emit this token.
        return;

        // Newlines are meaningful after other tokens.
      default:
        parser->skipNewlines = 0;
        return;
    }
  }
}

void readRawToken(Parser* parser)
{
  while (peekChar(parser) != '\0')
  {
    parser->tokenStart = parser->currentChar;

    char c = advanceChar(parser);
    switch (c)
    {
      case '(': makeToken(parser, TOKEN_LEFT_PAREN); return;
      case ')': makeToken(parser, TOKEN_RIGHT_PAREN); return;
      case '[': makeToken(parser, TOKEN_LEFT_BRACKET); return;
      case ']': makeToken(parser, TOKEN_RIGHT_BRACKET); return;
      case '{': makeToken(parser, TOKEN_LEFT_BRACE); return;
      case '}': makeToken(parser, TOKEN_RIGHT_BRACE); return;
      case ':': makeToken(parser, TOKEN_COLON); return;
      case '.': makeToken(parser, TOKEN_DOT); return;
      case ',': makeToken(parser, TOKEN_COMMA); return;
      case '*': makeToken(parser, TOKEN_STAR); return;
      case '/':
        if (peekChar(parser) == '/')
        {
          skipLineComment(parser);
          break;
        }
        makeToken(parser, TOKEN_SLASH);
        return;

      case '%': makeToken(parser, TOKEN_PERCENT); return;
      case '+': makeToken(parser, TOKEN_PLUS); return;
      case '-':
        if (isDigit(peekChar(parser)))
        {
          readNumber(parser);
        }
        else
        {
          makeToken(parser, TOKEN_MINUS);
        }
        return;

      case '|': makeToken(parser, TOKEN_PIPE); return;
      case '&': makeToken(parser, TOKEN_AMP); return;
      case '=':
        if (peekChar(parser) == '=')
        {
          advanceChar(parser);
          makeToken(parser, TOKEN_EQEQ);
        }
        else
        {
          makeToken(parser, TOKEN_EQ);
        }
        return;

      case '<':
        if (peekChar(parser) == '=')
        {
          advanceChar(parser);
          makeToken(parser, TOKEN_LTEQ);
        }
        else
        {
          makeToken(parser, TOKEN_LT);
        }
        return;

      case '>':
        if (peekChar(parser) == '=')
        {
          advanceChar(parser);
          makeToken(parser, TOKEN_GTEQ);
        }
        else
        {
          makeToken(parser, TOKEN_GT);
        }
        return;

      case '!':
        if (peekChar(parser) == '=')
        {
          advanceChar(parser);
          makeToken(parser, TOKEN_BANGEQ);
        }
        else
        {
          makeToken(parser, TOKEN_BANG);
        }
        return;

      case '\n': makeToken(parser, TOKEN_LINE); return;

      case ' ': skipWhitespace(parser); break;
      case '"': readString(parser); return;

      default:
        if (isName(c))
        {
          readName(parser);
        }
        else if (isDigit(c))
        {
          readNumber(parser);
        }
        else
        {
          makeToken(parser, TOKEN_ERROR);
        }
        return;
    }
  }

  // If we get here, we're out of source, so just make EOF tokens.
  parser->tokenStart = parser->currentChar;
  makeToken(parser, TOKEN_EOF);
}

void readName(Parser* parser)
{
  // TODO(bob): Handle EOF.
  while (isName(peekChar(parser)) || isDigit(peekChar(parser)))
  {
    advanceChar(parser);
  }

  TokenType type = TOKEN_NAME;

  if (isKeyword(parser, "class")) type = TOKEN_CLASS;
  if (isKeyword(parser, "meta")) type = TOKEN_META;
  if (isKeyword(parser, "var")) type = TOKEN_VAR;

  makeToken(parser, type);
}

int isKeyword(Parser* parser, const char* keyword)
{
  size_t length = parser->currentChar - parser->tokenStart;
  size_t keywordLength = strlen(keyword);
  return length == keywordLength &&
         strncmp(parser->source + parser->tokenStart, keyword, length) == 0;
}

void readNumber(Parser* parser)
{
  // TODO(bob): Floating point, hex, scientific, etc.
  while (isDigit(peekChar(parser))) advanceChar(parser);

  makeToken(parser, TOKEN_NUMBER);
}

void readString(Parser* parser)
{
  // TODO(bob): Escape sequences, EOL, EOF, etc.
  while (advanceChar(parser) != '"');

  makeToken(parser, TOKEN_STRING);
}

void skipLineComment(Parser* parser)
{
  while (peekChar(parser) != '\n' && peekChar(parser) != '\0')
  {
    advanceChar(parser);
  }
}

void skipWhitespace(Parser* parser)
{
  while (peekChar(parser) == ' ') advanceChar(parser);
}

int isName(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int isDigit(char c)
{
  return c >= '0' && c <= '9';
}

char advanceChar(Parser* parser)
{
  char c = peekChar(parser);
  parser->currentChar++;
  return c;
}

char peekChar(Parser* parser)
{
  return parser->source[parser->currentChar];
}

void makeToken(Parser* parser, TokenType type)
{
  parser->current.type = type;
  parser->current.start = parser->tokenStart;
  parser->current.end = parser->currentChar;
}

void initCompiler(Compiler* compiler, Parser* parser,
                         Compiler* parent)
{
  compiler->parser = parser;
  compiler->parent = parent;
  compiler->numCodes = 0;
  initSymbolTable(&compiler->locals);

  compiler->block = makeBlock();
  // TODO(bob): Hack! make variable sized.
  compiler->block->bytecode = malloc(sizeof(Code) * 1024);

  // TODO(bob): Hack! make variable sized.
  compiler->block->constants = malloc(sizeof(Value) * 256);
  compiler->block->numConstants = 0;
}

void emit(Compiler* compiler, Code code)
{
  compiler->block->bytecode[compiler->numCodes++] = code;
}

// Adds the previous token's text to the symbol table and returns its index.
int internSymbol(Compiler* compiler)
{
  return ensureSymbol(&compiler->parser->vm->symbols,
      compiler->parser->source + compiler->parser->previous.start,
      compiler->parser->previous.end - compiler->parser->previous.start);
}

void error(Compiler* compiler, const char* format, ...)
{
  compiler->parser->hasError = 1;
  printf("Compile error on '");

  for (int i = compiler->parser->previous.start;
       i < compiler->parser->previous.end; i++)
  {
    putchar(compiler->parser->source[i]);
  }

  printf("': ");

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}
