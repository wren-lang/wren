#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "compiler.h"

#define MAX_NAME 256

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
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FN,
  TOKEN_IF,
  TOKEN_META,
  TOKEN_NULL,
  TOKEN_TRUE,
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

  // The beginning of the token as an offset of characters in the source.
  int start;

  // The offset of the character immediately following the end of the token.
  int end;

  // The 1-based line where the token appears.
  int line;
} Token;

typedef struct
{
  VM* vm;

  const char* source;

  // The index in source of the beginning of the currently-being-lexed token.
  int tokenStart;

  // The position of the current character being lexed.
  int currentChar;

  // The 1-based line number of [currentChar].
  int currentLine;

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

  // The function being compiled.
  ObjFn* fn;
  int numCodes;

  // Symbol table for declared local variables in this block.
  SymbolTable locals;
} Compiler;

typedef void (*ParseFn)(Compiler*);

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

typedef struct
{
  ParseFn prefix;
  ParseFn infix;
  int precedence;
  const char* name;
} ParseRule;

static void initCompiler(Compiler* compiler, Parser* parser, Compiler* parent);

static ObjFn* compileFunction(Parser* parser, Compiler* parent,
                              TokenType endToken);

static int addConstant(Compiler* compiler, Value constant);

// Parses a name token and defines a variable in the current scope with that
// name. Returns its symbol.
static int defineName(Compiler* compiler);

// Stores a variable with the previously defined symbol in the current scope.
static void storeVariable(Compiler* compiler, int symbol);

// Adds the previous token's text to the symbol table and returns its index.
static int internSymbol(Compiler* compiler);

// Emits one bytecode instruction or argument.
static int emit(Compiler* compiler, Code code);

// Outputs a compile or syntax error.
static void error(Compiler* compiler, const char* format, ...);

// Parsing
// -------

static void statement(Compiler* compiler);
static void expression(Compiler* compiler);
static void parsePrecedence(Compiler* compiler, int precedence);

static void grouping(Compiler* compiler);
static void boolean(Compiler* compiler);
static void function(Compiler* compiler);
static void name(Compiler* compiler);
static void null(Compiler* compiler);
static void number(Compiler* compiler);
static void string(Compiler* compiler);

static void call(Compiler* compiler);
static void infixOp(Compiler* compiler);

static TokenType peek(Compiler* compiler);
static int match(Compiler* compiler, TokenType expected);
static void consume(Compiler* compiler, TokenType expected);

// Lexing
// ------

// Lex the next token in the source file and store it in parser.current. Omits
// newlines that aren't meaningful.
static void nextToken(Parser* parser);

// Lex the next token and store it in parser.current. Does not do any newline
// filtering.
static void readRawToken(Parser* parser);

// Finishes lexing an identifier. Handles reserved words.
static void readName(Parser* parser);

// Finishes lexing a number literal.
static void readNumber(Parser* parser);

// Finishes lexing a string literal.
static void readString(Parser* parser);

// Skips the rest of the current line.
static void skipLineComment(Parser* parser);

// Skips forward until a non-whitespace character is reached.
static void skipWhitespace(Parser* parser);

// Returns non-zero if the current token's text matches [keyword].
static int isKeyword(Parser* parser, const char* keyword);

// Returns non-zero if [c] is a valid (non-initial) identifier character.
static int isName(char c);

// Returns non-zero if [c] is a digit.
static int isDigit(char c);

// Advances the parser forward one character.
static char nextChar(Parser* parser);

// Returns the current character the parser is sitting on.
static char peekChar(Parser* parser);

// Sets the parser's current token to the given [type] and current character
// range.
static void makeToken(Parser* parser, TokenType type);

#define UNUSED { NULL, NULL, PREC_NONE, NULL }
#define PREFIX(fn) { fn, NULL, PREC_NONE, NULL }
#define INFIX(precedence, fn) { NULL, fn, precedence, NULL }
#define INFIX_OPERATOR(precendence, name) \
    { NULL, infixOp, precendence, name }

ParseRule rules[] =
{
  /* TOKEN_LEFT_PAREN    */ PREFIX(grouping),
  /* TOKEN_RIGHT_PAREN   */ UNUSED,
  /* TOKEN_LEFT_BRACKET  */ UNUSED,
  /* TOKEN_RIGHT_BRACKET */ UNUSED,
  /* TOKEN_LEFT_BRACE    */ UNUSED,
  /* TOKEN_RIGHT_BRACE   */ UNUSED,
  /* TOKEN_COLON         */ UNUSED,
  /* TOKEN_DOT           */ INFIX(PREC_CALL, call),
  /* TOKEN_COMMA         */ UNUSED,
  /* TOKEN_STAR          */ INFIX_OPERATOR(PREC_FACTOR, "* "),
  /* TOKEN_SLASH         */ INFIX_OPERATOR(PREC_FACTOR, "/ "),
  /* TOKEN_PERCENT       */ INFIX_OPERATOR(PREC_TERM, "% "),
  /* TOKEN_PLUS          */ INFIX_OPERATOR(PREC_TERM, "+ "),
  /* TOKEN_MINUS         */ INFIX_OPERATOR(PREC_TERM, "- "),
  /* TOKEN_PIPE          */ UNUSED,
  /* TOKEN_AMP           */ UNUSED,
  /* TOKEN_BANG          */ UNUSED,
  /* TOKEN_EQ            */ UNUSED,
  /* TOKEN_LT            */ INFIX_OPERATOR(PREC_COMPARISON, "< "),
  /* TOKEN_GT            */ INFIX_OPERATOR(PREC_COMPARISON, "> "),
  /* TOKEN_LTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, "<= "),
  /* TOKEN_GTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, ">= "),
  /* TOKEN_EQEQ          */ INFIX_OPERATOR(PREC_EQUALITY, "== "),
  /* TOKEN_BANGEQ        */ INFIX_OPERATOR(PREC_EQUALITY, "!= "),
  /* TOKEN_CLASS         */ UNUSED,
  /* TOKEN_ELSE          */ UNUSED,
  /* TOKEN_FALSE         */ PREFIX(boolean),
  /* TOKEN_FN            */ PREFIX(function),
  /* TOKEN_IF            */ UNUSED,
  /* TOKEN_META          */ UNUSED,
  /* TOKEN_NULL          */ PREFIX(null),
  /* TOKEN_TRUE          */ PREFIX(boolean),
  /* TOKEN_VAR           */ UNUSED,
  /* TOKEN_NAME          */ PREFIX(name),
  /* TOKEN_NUMBER        */ PREFIX(number),
  /* TOKEN_STRING        */ PREFIX(string),
  /* TOKEN_LINE          */ UNUSED,
  /* TOKEN_ERROR         */ UNUSED,
  /* TOKEN_EOF           */ UNUSED
};

ObjFn* compile(VM* vm, const char* source)
{
  Parser parser;
  parser.vm = vm;
  parser.source = source;
  parser.hasError = 0;

  // Ignore leading newlines.
  parser.skipNewlines = 1;

  parser.tokenStart = 0;
  parser.currentChar = 0;
  parser.currentLine = 1;

  // Zero-init the current token. This will get copied to previous when
  // advance() is called below.
  parser.current.type = TOKEN_EOF;
  parser.current.start = 0;
  parser.current.end = 0;
  parser.current.line = 0;

  // Read the first token.
  nextToken(&parser);

  return compileFunction(&parser, NULL, TOKEN_EOF);
}

void initCompiler(Compiler* compiler, Parser* parser,
                  Compiler* parent)
{
  compiler->parser = parser;
  compiler->parent = parent;
  compiler->numCodes = 0;
  initSymbolTable(&compiler->locals);

  compiler->fn = makeFunction();
  // TODO(bob): Hack! make variable sized.
  compiler->fn->bytecode = malloc(sizeof(Code) * 1024);

  // TODO(bob): Hack! make variable sized.
  compiler->fn->constants = malloc(sizeof(Value) * 256);
  compiler->fn->numConstants = 0;
}

ObjFn* compileFunction(Parser* parser, Compiler* parent, TokenType endToken)
{
  Compiler compiler;
  initCompiler(&compiler, parser, parent);

  for (;;)
  {
    statement(&compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(&compiler, TOKEN_LINE))
    {
      consume(&compiler, endToken);
      break;
    }

    if (match(&compiler, endToken)) break;

    // Discard the result of the previous expression.
    emit(&compiler, CODE_POP);
  }

  emit(&compiler, CODE_END);

  compiler.fn->numLocals = compiler.locals.count;

  return parser->hasError ? NULL : compiler.fn;
}

int addConstant(Compiler* compiler, Value constant)
{
  compiler->fn->constants[compiler->fn->numConstants++] = constant;
  return compiler->fn->numConstants - 1;
}

int defineName(Compiler* compiler)
{
  consume(compiler, TOKEN_NAME);

  SymbolTable* symbols;
  if (compiler->parent)
  {
    // Nested block, so this is a local variable.
    symbols = &compiler->locals;
  }
  else
  {
    // Top level global variable.
    symbols = &compiler->parser->vm->globalSymbols;
  }

  int symbol = addSymbol(symbols,
      compiler->parser->source + compiler->parser->previous.start,
      compiler->parser->previous.end - compiler->parser->previous.start);

  if (symbol == -1)
  {
    error(compiler, "Variable is already defined.");
  }

  return symbol;
}

void storeVariable(Compiler* compiler, int symbol)
{
  emit(compiler, compiler->parent ? CODE_STORE_LOCAL : CODE_STORE_GLOBAL);
  emit(compiler, symbol);
}

int internSymbol(Compiler* compiler)
{
  return ensureSymbol(&compiler->parser->vm->symbols,
      compiler->parser->source + compiler->parser->previous.start,
      compiler->parser->previous.end - compiler->parser->previous.start);
}

int emit(Compiler* compiler, Code code)
{
  compiler->fn->bytecode[compiler->numCodes++] = code;
  return compiler->numCodes - 1;
}

void error(Compiler* compiler, const char* format, ...)
{
  compiler->parser->hasError = 1;
  fprintf(stderr, "[Line %d] Error on '", compiler->parser->previous.line);

  for (int i = compiler->parser->previous.start;
       i < compiler->parser->previous.end; i++)
  {
    putc(compiler->parser->source[i], stderr);
  }

  fprintf(stderr, "': ");

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}

void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_CLASS))
  {
    int symbol = defineName(compiler);

    // Create the empty class.
    emit(compiler, CODE_CLASS);

    // Store it in its name.
    storeVariable(compiler, symbol);

    // Compile the method definitions.
    consume(compiler, TOKEN_LEFT_BRACE);

    while (!match(compiler, TOKEN_RIGHT_BRACE))
    {
      // Method name.
      consume(compiler, TOKEN_NAME);
      int symbol = internSymbol(compiler);

      consume(compiler, TOKEN_LEFT_BRACE);
      ObjFn* method = compileFunction(compiler->parser, compiler,
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
    int symbol = defineName(compiler);

    // TODO(bob): Allow uninitialized vars?
    consume(compiler, TOKEN_EQ);

    // Compile the initializer.
    expression(compiler);

    storeVariable(compiler, symbol);
    return;
  }

  // Statement expression.
  expression(compiler);
}

void expression(Compiler* compiler)
{
  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN);
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN);

    // TODO(bob): Block bodies.
    // Compile the then branch.
    emit(compiler, CODE_JUMP_IF);

    // Emit a placeholder. We'll patch it when we know what to jump to.
    int ifJump = emit(compiler, 255);

    expression(compiler);

    // Jump over the else branch when the if branch is taken.
    emit(compiler, CODE_JUMP);

    // Emit a placeholder. We'll patch it when we know what to jump to.
    int elseJump = emit(compiler, 255);

    // Patch the jump.
    compiler->fn->bytecode[ifJump] = compiler->numCodes - ifJump - 1;

    // Compile the else branch if there is one.
    if (match(compiler, TOKEN_ELSE))
    {
      // TODO(bob): Block bodies.
      expression(compiler);
    }
    else
    {
      // Just default to null.
      emit(compiler, CODE_NULL);
    }

    // Patch the jump over the else.
    compiler->fn->bytecode[elseJump] = compiler->numCodes - elseJump - 1;
    return;
  }
  
  return parsePrecedence(compiler, PREC_LOWEST);
}

void parsePrecedence(Compiler* compiler, int precedence)
{
  nextToken(compiler->parser);
  ParseFn prefix = rules[compiler->parser->previous.type].prefix;

  if (prefix == NULL)
  {
    // TODO(bob): Handle error better.
    error(compiler, "No prefix parser.");
    return;
  }

  prefix(compiler);

  while (precedence <= rules[compiler->parser->current.type].precedence)
  {
    nextToken(compiler->parser);
    ParseFn infix = rules[compiler->parser->previous.type].infix;
    infix(compiler);
  }
}

void grouping(Compiler* compiler)
{
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN);
}

void boolean(Compiler* compiler)
{
  if (compiler->parser->previous.type == TOKEN_FALSE)
  {
    emit(compiler, CODE_FALSE);
  }
  else
  {
    emit(compiler, CODE_TRUE);
  }
}

void function(Compiler* compiler)
{
  // TODO(bob): Copied from compileFunction(). Unify?
  Compiler fnCompiler;
  initCompiler(&fnCompiler, compiler->parser, compiler);

  if (match(&fnCompiler, TOKEN_LEFT_BRACE))
  {
    // Block body.
    for (;;)
    {
      statement(&fnCompiler);

      // If there is no newline, it must be the end of the block on the same line.
      if (!match(&fnCompiler, TOKEN_LINE))
      {
        consume(&fnCompiler, TOKEN_RIGHT_BRACE);
        break;
      }

      if (match(&fnCompiler, TOKEN_RIGHT_BRACE)) break;

      // Discard the result of the previous expression.
      emit(&fnCompiler, CODE_POP);
    }
  }
  else
  {
    // Single expression body.
    expression(&fnCompiler);
  }

  emit(&fnCompiler, CODE_END);
  fnCompiler.fn->numLocals = fnCompiler.locals.count;

  // Add the function to the constant table.
  compiler->fn->constants[compiler->fn->numConstants++] = (Value)fnCompiler.fn;

  // Compile the code to load it.
  emit(compiler, CODE_CONSTANT);
  emit(compiler, compiler->fn->numConstants - 1);
}

void name(Compiler* compiler)
{
  // See if it's a local in this scope.
  int local = findSymbol(&compiler->locals,
      compiler->parser->source + compiler->parser->previous.start,
      compiler->parser->previous.end - compiler->parser->previous.start);
  if (local != -1)
  {
    emit(compiler, CODE_LOAD_LOCAL);
    emit(compiler, local);
    return;
  }

  // TODO(bob): Look up names in outer local scopes.

  // See if it's a global variable.
  int global = findSymbol(&compiler->parser->vm->globalSymbols,
      compiler->parser->source + compiler->parser->previous.start,
      compiler->parser->previous.end - compiler->parser->previous.start);
  if (global != -1)
  {
    emit(compiler, CODE_LOAD_GLOBAL);
    emit(compiler, global);
    return;
  }

  // TODO(bob): Look for names in outer scopes.
  error(compiler, "Undefined variable.");
}

void null(Compiler* compiler)
{
  emit(compiler, CODE_NULL);
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

void string(Compiler* compiler)
{
  Token* token = &compiler->parser->previous;

  // TODO(bob): Handle escaping.

  // Copy the string to the heap.
  // Strip the surrounding "" off.
  size_t length = token->end - token->start - 2;
  char* text = malloc(length + 1);
  strncpy(text, compiler->parser->source + token->start + 1, length);
  text[length] = '\0';

  // Define a constant for the literal.
  int constant = addConstant(compiler, (Value)makeString(text));

  // Compile the code to load the constant.
  emit(compiler, CODE_CONSTANT);
  emit(compiler, constant);
}

// Method calls like:
//
// foo.bar
// foo.bar(arg, arg)
// foo.bar { block } other { block }
// foo.bar(arg) nextPart { arg } lastBit
void call(Compiler* compiler)
{
  char name[MAX_NAME];
  int length = 0;
  int numArgs = 0;

  consume(compiler, TOKEN_NAME);

  // Build the method name. The mangled name includes all of the name parts
  // in a mixfix call as well as spaces for every argument.
  // So a method call like:
  //
  //   foo.bar(arg, arg) else { block } last
  //
  // Will have name: "bar  else last"

  // Compile all of the name parts.
  for (;;)
  {
    // Add the just-consumed part name to the method name.
    int partLength = compiler->parser->previous.end -
    compiler->parser->previous.start;
    strncpy(name + length,
            compiler->parser->source + compiler->parser->previous.start,
            partLength);
    length += partLength;
    // TODO(bob): Check for length overflow.

    // Parse the argument list, if any.
    if (match(compiler, TOKEN_LEFT_PAREN))
    {
      for (;;)
      {
        expression(compiler);

        numArgs++;

        // Add a space in the name for each argument. Lets us overload by
        // arity.
        name[length++] = ' ';

        if (!match(compiler, TOKEN_COMMA)) break;
      }
      consume(compiler, TOKEN_RIGHT_PAREN);

      // If there isn't another part name after the argument list, stop.
      if (!match(compiler, TOKEN_NAME)) break;
    }
    else
    {
      // If there isn't an argument list, we're done.
      break;
    }
  }

  int symbol = ensureSymbol(&compiler->parser->vm->symbols, name, length);

  // Compile the method call.
  emit(compiler, CODE_CALL_0 + numArgs);
  // TODO(bob): Handle > 10 args.
  emit(compiler, symbol);
}

void infixOp(Compiler* compiler)
{
  ParseRule* rule = &rules[compiler->parser->previous.type];

  // Compile the right-hand side.
  parsePrecedence(compiler, rule->precedence + 1);

  // Call the operator method on the left-hand side.
  int symbol = ensureSymbol(&compiler->parser->vm->symbols,
                            rule->name, strlen(rule->name));
  emit(compiler, CODE_CALL_1);
  emit(compiler, symbol);
}

TokenType peek(Compiler* compiler)
{
  return compiler->parser->current.type;
}

// TODO(bob): Make a bool type?
int match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return 0;

  nextToken(compiler->parser);
  return 1;
}

void consume(Compiler* compiler, TokenType expected)
{
  nextToken(compiler->parser);
  if (compiler->parser->previous.type != expected)
  {
    // TODO(bob): Better error.
    error(compiler, "Expected %d, got %d.\n", expected,
          compiler->parser->previous.type);
  }
}

void nextToken(Parser* parser)
{
  // TODO(bob): Check for EOF.
  parser->previous = parser->current;

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
      case TOKEN_ELSE:
      case TOKEN_IF:
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

    char c = nextChar(parser);
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
          nextChar(parser);
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
          nextChar(parser);
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
          nextChar(parser);
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
          nextChar(parser);
          makeToken(parser, TOKEN_BANGEQ);
        }
        else
        {
          makeToken(parser, TOKEN_BANG);
        }
        return;

      case '\n':
        parser->currentLine++;
        makeToken(parser, TOKEN_LINE);
        return;

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
          // TODO(bob): Handle error.
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
    nextChar(parser);
  }

  TokenType type = TOKEN_NAME;

  if (isKeyword(parser, "class")) type = TOKEN_CLASS;
  if (isKeyword(parser, "else")) type = TOKEN_ELSE;
  if (isKeyword(parser, "false")) type = TOKEN_FALSE;
  if (isKeyword(parser, "fn")) type = TOKEN_FN;
  if (isKeyword(parser, "if")) type = TOKEN_IF;
  if (isKeyword(parser, "meta")) type = TOKEN_META;
  if (isKeyword(parser, "null")) type = TOKEN_NULL;
  if (isKeyword(parser, "true")) type = TOKEN_TRUE;
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
  while (isDigit(peekChar(parser))) nextChar(parser);

  makeToken(parser, TOKEN_NUMBER);
}

void readString(Parser* parser)
{
  // TODO(bob): Escape sequences, EOL, EOF, etc.
  while (nextChar(parser) != '"');

  makeToken(parser, TOKEN_STRING);
}

void skipLineComment(Parser* parser)
{
  while (peekChar(parser) != '\n' && peekChar(parser) != '\0')
  {
    nextChar(parser);
  }
}

void skipWhitespace(Parser* parser)
{
  while (peekChar(parser) == ' ') nextChar(parser);
}

int isName(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int isDigit(char c)
{
  return c >= '0' && c <= '9';
}

char nextChar(Parser* parser)
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
  parser->current.line = parser->currentLine;
}

