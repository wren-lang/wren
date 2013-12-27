#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_vm.h"

// This is written in bottom-up order, so the tokenization comes first, then
// parsing/code generation. This minimizes the number of explicit forward
// declarations needed.

// The maximum number of arguments that can be passed to a method. Note that
// this limtation is hardcoded in other places in the VM, in particular, the
// `CODE_CALL_XX` instructions assume a certain maximum number.
#define MAX_PARAMETERS (16)

// The maximum number of local (i.e. non-global) variables that can be declared
// in a single function, method, or chunk of top level code. This is the
// maximum number of variables in scope at one time, and spans block scopes.
//
// Note that this limitation is also explicit in the bytecode. Since
// `CODE_LOAD_LOCAL` and `CODE_STORE_LOCAL` use a single argument byte to
// identify the local, only 256 can be in scope at one time.
#define MAX_LOCALS (256)

// The maximum number of upvalues (i.e. variables from enclosing functions)
// that a function can close over.
#define MAX_UPVALUES (256)

// The maximum number of distinct constants that a function can contain. This
// value is explicit in the bytecode since `CODE_CONSTANT` only takes a single
// argument.
#define MAX_CONSTANTS (256)

// The maximum name of a method, not including the signature. This is an
// arbitrary but enforced maximum just so we know how long the method name
// strings need to be in the parser.
#define MAX_METHOD_NAME (64)

// The maximum length of a method signature. This includes the name, and the
// extra spaces added to handle arity, and another byte to terminate the string.
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + MAX_PARAMETERS + 1)

// TODO: Get rid of this and use a growable buffer.
#define MAX_STRING (1024)

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
  TOKEN_DOTDOT,
  TOKEN_DOTDOTDOT,
  TOKEN_COMMA,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_PERCENT,
  TOKEN_PLUS,
  TOKEN_MINUS,
  TOKEN_PIPE,
  TOKEN_PIPEPIPE,
  TOKEN_AMP,
  TOKEN_AMPAMP,
  TOKEN_BANG,
  TOKEN_TILDE,
  TOKEN_EQ,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_LTEQ,
  TOKEN_GTEQ,
  TOKEN_EQEQ,
  TOKEN_BANGEQ,

  TOKEN_BREAK,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FN,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_IN,
  TOKEN_IS,
  TOKEN_NEW,
  TOKEN_NULL,
  TOKEN_RETURN,
  TOKEN_STATIC,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,

  TOKEN_FIELD,
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

  // The beginning of the token, pointing directly into the source.
  const char* start;

  // The length of the token in characters.
  int length;

  // The 1-based line where the token appears.
  int line;
} Token;

// A growable byte buffer.
typedef struct
{
  uint8_t* bytes;
  int length;
  int capacity;
} Buffer;

typedef struct
{
  WrenVM* vm;

  const char* source;

  // The beginning of the currently-being-lexed token in [source].
  const char* tokenStart;

  // The current character being lexed in [source].
  const char* currentChar;

  // The 1-based line number of [currentChar].
  int currentLine;

  // The most recently lexed token.
  Token current;

  // The most recently consumed/advanced token.
  Token previous;

  // If subsequent newline tokens should be discarded.
  bool skipNewlines;

  // If a syntax or compile error has occurred.
  bool hasError;

  // A buffer for the unescaped text of the current token if it's a string
  // literal. Unlike the raw token, this will have escape sequences translated
  // to their literal equivalent.
  Buffer string;
} Parser;

typedef struct
{
  // The name of the local variable. This points directly into the original
  // source code string.
  const char* name;

  // The length of the local variable's name.
  int length;

  // The depth in the scope chain that this variable was declared at. Zero is
  // the outermost scope--parameters for a method, or the first local block in
  // top level code. One is the scope within that, etc.
  int depth;

  // If this local variable is being used as an upvalue.
  bool isUpvalue;
} Local;

typedef struct
{
  // True if this upvalue is capturing a local variable from the enclosing
  // function. False if it's capturing an upvalue.
  bool isLocal;

  // The index of the local or upvalue being captured in the enclosing function.
  int index;
} CompilerUpvalue;

struct sCompiler
{
  Parser* parser;

  // The compiler for the function enclosing this one, or NULL if it's the
  // top level.
  struct sCompiler* parent;

  // The growable buffer of code that's been compiled so far.
  Buffer bytecode;

  ObjList* constants;

  // Symbol table for the fields of the nearest enclosing class, or NULL if not
  // currently inside a class.
  SymbolTable* fields;

  // The number of local variables currently in scope.
  int numLocals;

  // The current level of block scope nesting, where zero is no nesting. A -1
  // here means top-level code is being compiled and there is no block scope
  // in effect at all. Any variables declared will be global.
  int scopeDepth;

  // Index of the first instruction of the body of the innermost loop currently
  // being compiled. Will be -1 if not currently inside a loop.
  int loopBody;

  // The name of the method this compiler is compiling, or NULL if this
  // compiler is not for a method. Note that this is just the bare method name,
  // and not its full signature.
  const char* methodName;

  // The length of the method name being compiled.
  int methodLength;

  // The currently in scope local variables.
  Local locals[MAX_LOCALS];

  // The upvalues that this function has captured from outer scopes. The count
  // of them is stored in [numUpvalues].
  CompilerUpvalue upvalues[MAX_UPVALUES];

  int numUpvalues;
};

// Outputs a compile or syntax error. This also marks the compilation as having
// an error, which ensures that the resulting code will be discarded and never
// run. This means that after calling lexError(), it's fine to generate whatever
// invalid bytecode you want since it won't be used.
static void lexError(Parser* parser, const char* format, ...)
{
  parser->hasError = true;

  fprintf(stderr, "[Line %d] Error: ", parser->currentLine);

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}

// Outputs a compile or syntax error. This also marks the compilation as having
// an error, which ensures that the resulting code will be discarded and never
// run. This means that after calling error(), it's fine to generate whatever
// invalid bytecode you want since it won't be used.
//
// You'll note that most places that call error() continue to parse and compile
// after that. That's so that we can try to find as many compilation errors in
// one pass as possible instead of just bailing at the first one.
static void error(Compiler* compiler, const char* format, ...)
{
  compiler->parser->hasError = true;

  Token* token = &compiler->parser->previous;
  fprintf(stderr, "[Line %d] Error on ", token->line);
  if (token->type == TOKEN_LINE)
  {
    // Don't print the newline itself since that looks wonky.
    fprintf(stderr, "newline: ");
  }
  else
  {
    fprintf(stderr, "'%.*s': ", token->length, token->start);
  }

  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);

  fprintf(stderr, "\n");
}

// Adds [constant] to the constant pool and returns its index.
static int addConstant(Compiler* compiler, Value constant)
{
  // See if an equivalent constant has already been added.
  for (int i = 0; i < compiler->constants->count; i++)
  {
    // TODO: wrenValuesEqual doesn't check for string equality. Check for that
    // explicitly here or intern strings globally or something.
    if (wrenValuesEqual(compiler->constants->elements[i], constant)) return i;
  }

  if (compiler->constants->count < MAX_CONSTANTS)
  {
    wrenListAdd(compiler->parser->vm, compiler->constants, constant);
  }
  else
  {
    error(compiler, "A function may only contain %d unique constants.",
          MAX_CONSTANTS);
  }

  return compiler->constants->count - 1;
}

static void initBuffer(WrenVM* vm, Buffer* buffer)
{
  buffer->bytes = NULL;
  buffer->capacity = 0;
  buffer->length = 0;
}

static void ensureBufferCapacity(WrenVM* vm, Buffer* buffer, int needed)
{
  // Do nothing if already big enough.
  if (buffer->capacity >= needed) return;

  // Give it an initial bump, then double each time.
  int capacity = buffer->capacity == 0 ? 2 : buffer->capacity * 2;
  buffer->bytes = wrenReallocate(vm, buffer->bytes,
                                 buffer->capacity, capacity);
  // TODO: Handle allocation failure.
  buffer->capacity = capacity;
}

static void writeBuffer(WrenVM* vm, Buffer* buffer, uint8_t byte)
{
  ensureBufferCapacity(vm, buffer, buffer->length + 1);
  buffer->bytes[buffer->length++] = byte;
}

static void freeBuffer(WrenVM* vm, Buffer* buffer)
{
  wrenReallocate(vm, buffer->bytes, buffer->capacity, 0);
  buffer->bytes = NULL;
  buffer->capacity = 0;
  buffer->length = 0;
}

// Initializes [compiler].
static void initCompiler(Compiler* compiler, Parser* parser, Compiler* parent,
                         const char* methodName, int methodLength)
{
  // TODO: Reorganize initialization and fields to be in same order as struct.
  compiler->parser = parser;
  compiler->parent = parent;
  compiler->loopBody = -1;
  compiler->methodName = methodName;
  compiler->methodLength = methodLength;

  initBuffer(parser->vm, &compiler->bytecode);

  // Initialize these to NULL before allocating them in case a GC gets
  // triggered in the middle of creating these.
  compiler->constants = NULL;

  wrenSetCompiler(parser->vm, compiler);

  // Create a growable list for the constants used by this function.
  compiler->constants = wrenNewList(parser->vm, 0);

  compiler->numUpvalues = 0;

  if (parent == NULL)
  {
    // Compiling top-level code, so the initial scope is global.
    compiler->scopeDepth = -1;
    compiler->numLocals = 0;
  }
  else
  {
    // Declare a fake local variable for the receiver so that it's slot in the
    // stack is taken. For methods, we call this "this", so that we can resolve
    // references to that like a normal variable. For functions, they have no
    // explicit "this". So we pick a bogus name. That way references to "this"
    // inside a function will try to walk up the parent chain to find a method
    // enclosing the function whose "this" we can close over.
    compiler->numLocals = 1;
    if (methodName != NULL)
    {
      compiler->locals[0].name = "this";
      compiler->locals[0].length = 4;
    }
    else
    {
      compiler->locals[0].name = NULL;
      compiler->locals[0].length = 0;
    }
    compiler->locals[0].depth = -1;
    compiler->locals[0].isUpvalue = false;

    // The initial scope for function or method is a local scope.
    compiler->scopeDepth = 0;
  }

  // Propagate the enclosing class downwards.
  compiler->fields = parent != NULL ? parent->fields :  NULL;
}

// Lexing ----------------------------------------------------------------------

// Returns true if [c] is a valid (non-initial) identifier character.
static bool isName(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Returns true if [c] is a digit.
static bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

// Returns the current character the parser is sitting on.
static char peekChar(Parser* parser)
{
  return *parser->currentChar;
}

// Returns the character after the current character.
static char peekNextChar(Parser* parser)
{
  // If we're at the end of the source, don't read past it.
  if (peekChar(parser) == '\0') return '\0';
  return *(parser->currentChar + 1);
}

// Advances the parser forward one character.
static char nextChar(Parser* parser)
{
  char c = peekChar(parser);
  parser->currentChar++;
  if (c == '\n') parser->currentLine++;
  return c;
}

// Sets the parser's current token to the given [type] and current character
// range.
static void makeToken(Parser* parser, TokenType type)
{
  parser->current.type = type;
  parser->current.start = parser->tokenStart;
  parser->current.length = (int)(parser->currentChar - parser->tokenStart);
  parser->current.line = parser->currentLine;
}

// If the current character is [c], then consumes it and makes a token of type
// [two]. Otherwise makes a token of type [one].
static void twoCharToken(Parser* parser, char c, TokenType two, TokenType one)
{
  if (peekChar(parser) == c)
  {
    nextChar(parser);
    makeToken(parser, two);
    return;
  }

  makeToken(parser, one);
}

// Skips the rest of the current line.
static void skipLineComment(Parser* parser)
{
  while (peekChar(parser) != '\n' && peekChar(parser) != '\0')
  {
    nextChar(parser);
  }
}

// Skips the rest of a block comment.
static void skipBlockComment(Parser* parser)
{
  nextChar(parser); // The opening "*".

  int nesting = 1;
  while (nesting > 0)
  {
    if (peekChar(parser) == '\0')
    {
      lexError(parser, "Unterminated block comment.");
      return;
    }

    if (peekChar(parser) == '/' && peekNextChar(parser) == '*')
    {
      nextChar(parser);
      nextChar(parser);
      nesting++;
      continue;
    }

    if (peekChar(parser) == '*' && peekNextChar(parser) == '/')
    {
      nextChar(parser);
      nextChar(parser);
      nesting--;
      continue;
    }

    // Regular comment character.
    nextChar(parser);
  }
}

// Returns true if the current token's text matches [keyword].
static bool isKeyword(Parser* parser, const char* keyword)
{
  size_t length = parser->currentChar - parser->tokenStart;
  size_t keywordLength = strlen(keyword);
  return length == keywordLength &&
      strncmp(parser->tokenStart, keyword, length) == 0;
}

// Finishes lexing a number literal.
static void readNumber(Parser* parser)
{
  // TODO: Hex, scientific, etc.
  while (isDigit(peekChar(parser))) nextChar(parser);

  // See if it has a floating point. Make sure there is a digit after the "."
  // so we don't get confused by method calls on number literals.
  if (peekChar(parser) == '.' && isDigit(peekNextChar(parser)))
  {
    nextChar(parser);
    while (isDigit(peekChar(parser))) nextChar(parser);
  }

  makeToken(parser, TOKEN_NUMBER);
}

// Finishes lexing an identifier. Handles reserved words.
static void readName(Parser* parser, TokenType type)
{
  while (isName(peekChar(parser)) || isDigit(peekChar(parser)))
  {
    nextChar(parser);
  }

  if (isKeyword(parser, "break")) type = TOKEN_BREAK;
  if (isKeyword(parser, "class")) type = TOKEN_CLASS;
  if (isKeyword(parser, "else")) type = TOKEN_ELSE;
  if (isKeyword(parser, "false")) type = TOKEN_FALSE;
  if (isKeyword(parser, "fn")) type = TOKEN_FN;
  if (isKeyword(parser, "for")) type = TOKEN_FOR;
  if (isKeyword(parser, "if")) type = TOKEN_IF;
  if (isKeyword(parser, "in")) type = TOKEN_IN;
  if (isKeyword(parser, "is")) type = TOKEN_IS;
  if (isKeyword(parser, "new")) type = TOKEN_NEW;
  if (isKeyword(parser, "null")) type = TOKEN_NULL;
  if (isKeyword(parser, "return")) type = TOKEN_RETURN;
  if (isKeyword(parser, "static")) type = TOKEN_STATIC;
  if (isKeyword(parser, "super")) type = TOKEN_SUPER;
  if (isKeyword(parser, "this")) type = TOKEN_THIS;
  if (isKeyword(parser, "true")) type = TOKEN_TRUE;
  if (isKeyword(parser, "var")) type = TOKEN_VAR;
  if (isKeyword(parser, "while")) type = TOKEN_WHILE;

  makeToken(parser, type);
}

// Adds [c] to the current string literal being tokenized.
static void addStringChar(Parser* parser, char c)
{
  writeBuffer(parser->vm, &parser->string, c);
}

// Finishes lexing a string literal.
static void readString(Parser* parser)
{
  freeBuffer(parser->vm, &parser->string);

  for (;;)
  {
    char c = nextChar(parser);
    if (c == '"') break;

    if (c == '\\')
    {
      switch (nextChar(parser))
      {
        case '"':  addStringChar(parser, '"'); break;
        case '\\': addStringChar(parser, '\\'); break;
        case 'n':  addStringChar(parser, '\n'); break;
        case 't':  addStringChar(parser, '\t'); break;
        default:
          // TODO: Emit error token.
          break;
      }

      // TODO: Other escapes (\r, etc.), Unicode escape sequences.
    }
    else
    {
      addStringChar(parser, c);
    }
  }

  makeToken(parser, TOKEN_STRING);
}

// Lex the next token and store it in [parser.current]. Does not do any newline
// filtering.
static void readRawToken(Parser* parser)
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
      case ';': makeToken(parser, TOKEN_LINE); return;
      case ':': makeToken(parser, TOKEN_COLON); return;
      case '.':
        if (peekChar(parser) == '.')
        {
          nextChar(parser);
          if (peekChar(parser) == '.')
          {
            nextChar(parser);
            makeToken(parser, TOKEN_DOTDOTDOT);
            return;
          }

          makeToken(parser, TOKEN_DOTDOT);
          return;
        }

        makeToken(parser, TOKEN_DOT);
        return;

      case ',': makeToken(parser, TOKEN_COMMA); return;
      case '*': makeToken(parser, TOKEN_STAR); return;
      case '%': makeToken(parser, TOKEN_PERCENT); return;
      case '+': makeToken(parser, TOKEN_PLUS); return;
      case '~': makeToken(parser, TOKEN_TILDE); return;
      case '/':
        if (peekChar(parser) == '/')
        {
          skipLineComment(parser);
          break;
        }

        if (peekChar(parser) == '*')
        {
          skipBlockComment(parser);
          break;
        }

        makeToken(parser, TOKEN_SLASH);
        return;

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

      case '|':
        twoCharToken(parser, '|', TOKEN_PIPEPIPE, TOKEN_PIPE);
        return;

      case '&':
        twoCharToken(parser, '&', TOKEN_AMPAMP, TOKEN_AMP);
        return;

      case '=':
        twoCharToken(parser, '=', TOKEN_EQEQ, TOKEN_EQ);
        return;

      case '<':
        twoCharToken(parser, '=', TOKEN_LTEQ, TOKEN_LT);
        return;

      case '>':
        twoCharToken(parser, '=', TOKEN_GTEQ, TOKEN_GT);
        return;

      case '!':
        twoCharToken(parser, '=', TOKEN_BANGEQ, TOKEN_BANG);
        return;

      case '\n':
        makeToken(parser, TOKEN_LINE);
        return;

      case ' ':
        // Skip forward until we run out of whitespace.
        while (peekChar(parser) == ' ') nextChar(parser);
        break;

      case '"': readString(parser); return;
      case '_': readName(parser, TOKEN_FIELD); return;

      default:
        if (isName(c))
        {
          readName(parser, TOKEN_NAME);
        }
        else if (isDigit(c))
        {
          readNumber(parser);
        }
        else
        {
          lexError(parser, "Invalid character '%c'.", c);
        }
        return;
    }
  }

  // If we get here, we're out of source, so just make EOF tokens.
  parser->tokenStart = parser->currentChar;
  makeToken(parser, TOKEN_EOF);
}

// Lex the next token in the source file and store it in parser.current. Omits
// newlines that aren't meaningful.
static void nextToken(Parser* parser)
{
  parser->previous = parser->current;

  // If we are out of tokens, don't try to tokenize any more. We *do* still
  // copy the TOKEN_EOF to previous so that code that expects it to be consumed
  // will still work.
  if (parser->current.type == TOKEN_EOF) return;

  for (;;)
  {
    readRawToken(parser);

    switch (parser->current.type)
    {
      case TOKEN_LINE:
        if (!parser->skipNewlines)
        {
          // Collapse multiple newlines into one.
          parser->skipNewlines = true;

          // Emit this newline.
          return;
        }
        break;

        // Discard newlines after tokens that cannot end an expression.
      case TOKEN_LEFT_PAREN:
      case TOKEN_LEFT_BRACKET:
      case TOKEN_LEFT_BRACE:
      case TOKEN_DOT:
      case TOKEN_DOTDOT:
      case TOKEN_DOTDOTDOT:
      case TOKEN_COMMA:
      case TOKEN_STAR:
      case TOKEN_SLASH:
      case TOKEN_PERCENT:
      case TOKEN_PLUS:
      case TOKEN_MINUS:
      case TOKEN_PIPE:
      case TOKEN_PIPEPIPE:
      case TOKEN_AMP:
      case TOKEN_AMPAMP:
      case TOKEN_BANG:
      case TOKEN_TILDE:
      case TOKEN_EQ:
      case TOKEN_LT:
      case TOKEN_GT:
      case TOKEN_LTEQ:
      case TOKEN_GTEQ:
      case TOKEN_EQEQ:
      case TOKEN_BANGEQ:
      case TOKEN_CLASS:
      case TOKEN_ELSE:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_IN:
      case TOKEN_IS:
      case TOKEN_NEW:
      case TOKEN_STATIC:
      case TOKEN_SUPER:
      case TOKEN_VAR:
      case TOKEN_WHILE:
        parser->skipNewlines = true;

        // Emit this token.
        return;

        // Newlines are meaningful after other tokens.
      default:
        parser->skipNewlines = false;
        return;
    }
  }
}

// Parsing ---------------------------------------------------------------------

// Returns the type of the current token.
static TokenType peek(Compiler* compiler)
{
  return compiler->parser->current.type;
}

// Consumes the current token if its type is [expected]. Returns true if a
// token was consumed.
static bool match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return false;

  nextToken(compiler->parser);
  return true;
}

// Consumes the current token. Emits an error if its type is not [expected].
// Returns the consumed token.
static Token* consume(Compiler* compiler, TokenType expected,
                    const char* errorMessage)
{
  nextToken(compiler->parser);
  if (compiler->parser->previous.type != expected)
  {
    error(compiler, errorMessage);
  }

  return &compiler->parser->previous;
}

// Variables and scopes --------------------------------------------------------

// Emits one bytecode instruction or argument. Returns its index.
static int emit(Compiler* compiler, Code code)
{
  writeBuffer(compiler->parser->vm, &compiler->bytecode, code);
  return compiler->bytecode.length - 1;
}

// Emits one bytecode instruction followed by an argument. Returns the index of
// the argument in the bytecode.
static int emit1(Compiler* compiler, Code instruction, uint8_t arg)
{
  emit(compiler, instruction);
  return emit(compiler, arg);
}

// Create a new local variable with [name]. Assumes the current scope is local
// and the name is unique.
static int defineLocal(Compiler* compiler, const char* name, int length)
{
  Local* local = &compiler->locals[compiler->numLocals];
  local->name = name;
  local->length = length;
  local->depth = compiler->scopeDepth;
  local->isUpvalue = false;
  return compiler->numLocals++;
}

// Parses a name token and declares a variable in the current scope with that
// name. Returns its symbol.
static int declareVariable(Compiler* compiler)
{
  Token* token = consume(compiler, TOKEN_NAME, "Expected variable name.");

  // Top-level global scope.
  if (compiler->scopeDepth == -1)
  {
    SymbolTable* symbols = &compiler->parser->vm->globalSymbols;

    int symbol = addSymbol(symbols, token->start, token->length);
    if (symbol == -1)
    {
      error(compiler, "Global variable is already defined.");
    }

    return symbol;
  }

  // See if there is already a variable with this name declared in the current
  // scope. (Outer scopes are OK: those get shadowed.)
  for (int i = compiler->numLocals - 1; i >= 0; i--)
  {
    Local* local = &compiler->locals[i];

    // Once we escape this scope and hit an outer one, we can stop.
    if (local->depth < compiler->scopeDepth) break;

    if (local->length == token->length &&
        strncmp(local->name, token->start, token->length) == 0)
    {
      error(compiler, "Variable is already declared in this scope.");
      return i;
    }
  }

  if (compiler->numLocals == MAX_LOCALS)
  {
    error(compiler, "Cannot declare more than %d variables in one scope.",
          MAX_LOCALS);
    return -1;
  }

  return defineLocal(compiler, token->start, token->length);
}

// Stores a variable with the previously defined symbol in the current scope.
static void defineVariable(Compiler* compiler, int symbol)
{
  // Store the variable. If it's a local, the result of the initializer is
  // in the correct slot on the stack already so we're done.
  if (compiler->scopeDepth >= 0) return;

  // It's a global variable, so store the value in the global slot and then
  // discard the temporary for the initializer.
  emit1(compiler, CODE_STORE_GLOBAL, symbol);
  emit(compiler, CODE_POP);
}

// Starts a new local block scope.
static void pushScope(Compiler* compiler)
{
  compiler->scopeDepth++;
}

// Closes the last pushed block scope. This should only be called in a statement
// context where no temporaries are still on the stack.
static void popScope(Compiler* compiler)
{
  ASSERT(compiler->scopeDepth > -1, "Cannot pop top-level scope.");

  // Pop locals off the stack.
  while (compiler->numLocals > 0 &&
         compiler->locals[compiler->numLocals - 1].depth ==
            compiler->scopeDepth)
  {
    compiler->numLocals--;

    // If the local was closed over, make sure the upvalue gets closed when it
    // goes out of scope on the stack.
    if (compiler->locals[compiler->numLocals].isUpvalue)
    {
      emit(compiler, CODE_CLOSE_UPVALUE);
    }
    else
    {
      emit(compiler, CODE_POP);
    }
  }

  compiler->scopeDepth--;
}

// Attempts to look up the name in the local variables of [compiler]. If found,
// returns its index, otherwise returns -1.
static int resolveLocal(Compiler* compiler, const char* name, int length)
{
  // Look it up in the local scopes. Look in reverse order so that the most
  // nested variable is found first and shadows outer ones.
  for (int i = compiler->numLocals - 1; i >= 0; i--)
  {
    if (compiler->locals[i].length == length &&
        strncmp(name, compiler->locals[i].name, length) == 0)
    {
      return i;
    }
  }

  return -1;
}

// Adds an upvalue to [compiler]'s function with the given properties. Does not
// add one if an upvalue for that variable is already in the list. Returns the
// index of the uvpalue.
static int addUpvalue(Compiler* compiler, bool isLocal, int index)
{
  // Look for an existing one.
  for (int i = 0; i < compiler->numUpvalues; i++)
  {
    CompilerUpvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) return i;
  }

  // If we got here, it's a new upvalue.
  compiler->upvalues[compiler->numUpvalues].isLocal = isLocal;
  compiler->upvalues[compiler->numUpvalues].index = index;
  return compiler->numUpvalues++;
}

// Attempts to look up [name] in the functions enclosing the one being compiled
// by [compiler]. If found, it adds an upvalue for it to this compiler's list
// of upvalues (unless it's already in there) and returns its index. If not
// found, returns -1.
//
// If the name is found outside of the immediately enclosing function, this
// will flatten the closure and add upvalues to all of the intermediate
// functions so that it gets walked down to this one.
static int findUpvalue(Compiler* compiler, const char* name, int length)
{
  // If we are out of enclosing functions, it can't be an upvalue.
  if (compiler->parent == NULL)
  {
    return -1;
  }

  // See if it's a local variable in the immediately enclosing function.
  int local = resolveLocal(compiler->parent, name, length);
  if (local != -1)
  {
    // Mark the local as an upvalue so we know to close it when it goes out of
    // scope.
    compiler->parent->locals[local].isUpvalue = true;

    return addUpvalue(compiler, true, local);
  }

  // See if it's an upvalue in the immediately enclosing function. In other
  // words, if its a local variable in a non-immediately enclosing function.
  // This will "flatten" closures automatically: it will add upvalues to all
  // of the intermediate functions to get from the function where a local is
  // declared all the way into the possibly deeply nested function that is
  // closing over it.
  int upvalue = findUpvalue(compiler->parent, name, length);
  if (upvalue != -1)
  {
    return addUpvalue(compiler, false, upvalue);
  }

  // If we got here, we walked all the way up the parent chain and couldn't
  // find it.
  return -1;
}

// Look up [name] in the current scope to see what name it is bound to. Returns
// the index of the name either in global scope, local scope, or the enclosing
// function's upvalue list. Returns -1 if not found.
//
// Sets [loadInstruction] to the instruction needed to load the variable. Will
// be one of [CODE_LOAD_LOCAL], [CODE_LOAD_UPVALUE], or [CODE_LOAD_GLOBAL].
static int resolveName(Compiler* compiler, const char* name, int length,
                       Code* loadInstruction)
{
  // Look it up in the local scopes. Look in reverse order so that the most
  // nested variable is found first and shadows outer ones.
  *loadInstruction = CODE_LOAD_LOCAL;
  int local = resolveLocal(compiler, name, length);
  if (local != -1) return local;

  // If we got here, it's not a local, so lets see if we are closing over an
  // outer local.
  *loadInstruction = CODE_LOAD_UPVALUE;
  int upvalue = findUpvalue(compiler, name, length);
  if (upvalue != -1) return upvalue;

  // If we got here, it wasn't in a local scope, so try the global scope.
  *loadInstruction = CODE_LOAD_GLOBAL;
  return findSymbol(&compiler->parser->vm->globalSymbols, name, length);
}

// Copies the identifier from the previously consumed `TOKEN_NAME` into [name],
// which should point to a buffer large enough to contain it. Returns the
// length of the name.
static int copyName(Compiler* compiler, char* name)
{
  Token* token = &compiler->parser->previous;
  int length = token->length;

  if (length > MAX_METHOD_NAME)
  {
    error(compiler, "Method names cannot be longer than %d characters.",
          MAX_METHOD_NAME);
    length = MAX_METHOD_NAME;
  }

  strncpy(name, token->start, length);
  return length;
}

// Finishes [compiler], which is compiling a function, method, or chunk of top
// level code. If there is a parent compiler, then this emits code in the
// parent compiler to load the resulting function.
static ObjFn* endCompiler(Compiler* compiler)
{
  // If we hit an error, don't bother creating the function since it's borked
  // anyway.
  if (compiler->parser->hasError)
  {
    // Free the bytecode since it won't be used.
    freeBuffer(compiler->parser->vm, &compiler->bytecode);

    return NULL;
  }

  // Mark the end of the bytecode. Since it may contain multiple early returns,
  // we can't rely on CODE_RETURN to tell us we're at the end.
  emit(compiler, CODE_END);

  // Create a function object for the code we just compiled.
  ObjFn* fn = wrenNewFunction(compiler->parser->vm,
                              compiler->constants->elements,
                              compiler->constants->count,
                              compiler->numUpvalues,
                              compiler->bytecode.bytes,
                              compiler->bytecode.length);
  PinnedObj pinned;
  pinObj(compiler->parser->vm, (Obj*)fn, &pinned);

  // In the function that contains this one, load the resulting function object.
  if (compiler->parent != NULL)
  {
    int constant = addConstant(compiler->parent, OBJ_VAL(fn));

    // If the function has no upvalues, we don't need to create a closure.
    // We can just load and run the function directly.
    if (compiler->numUpvalues == 0)
    {
      emit1(compiler->parent, CODE_CONSTANT, constant);
    }
    else
    {
      // Capture the upvalues in the new closure object.
      emit1(compiler->parent, CODE_CLOSURE, constant);

      // Emit arguments for each upvalue to know whether to capture a local or
      // an upvalue.
      // TODO: Do something more efficient here?
      for (int i = 0; i < compiler->numUpvalues; i++)
      {
        emit1(compiler->parent, compiler->upvalues[i].isLocal ? 1 : 0,
              compiler->upvalues[i].index);
      }
    }
  }

  // Pop this compiler off the stack.
  wrenSetCompiler(compiler->parser->vm, compiler->parent);

  unpinObj(compiler->parser->vm);

  return fn;
}

// Grammar ---------------------------------------------------------------------

typedef enum
{
  PREC_NONE,
  PREC_LOWEST,
  PREC_ASSIGNMENT, // =
  PREC_LOGIC,      // && ||
  PREC_IS,         // is
  PREC_EQUALITY,   // == !=
  PREC_COMPARISON, // < > <= >=
  PREC_RANGE,      // .. ...
  PREC_BITWISE,    // | &
  PREC_TERM,       // + -
  PREC_FACTOR,     // * / %
  PREC_UNARY,      // unary - ! ~
  PREC_CALL        // . () []
} Precedence;

// Forward declarations since the grammar is recursive.
static void expression(Compiler* compiler);
static void statement(Compiler* compiler);
static void definition(Compiler* compiler);
static void parsePrecedence(Compiler* compiler, bool allowAssignment,
                            Precedence precedence);

typedef void (*GrammarFn)(Compiler*, bool allowAssignment);

typedef void (*SignatureFn)(Compiler* compiler, char* name, int* length);

typedef struct
{
  GrammarFn prefix;
  GrammarFn infix;
  SignatureFn method;
  Precedence precedence;
  const char* name;
} GrammarRule;

GrammarRule rules[];

// Replaces the placeholder argument for a previous CODE_JUMP or CODE_JUMP_IF
// instruction with an offset that jumps to the current end of bytecode.
static void patchJump(Compiler* compiler, int offset)
{
  compiler->bytecode.bytes[offset] = compiler->bytecode.length - offset - 1;
}

// Parses a block body, after the initial "{" has been consumed.
static void finishBlock(Compiler* compiler)
{
  for (;;)
  {
    definition(compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(compiler, TOKEN_LINE))
    {
      consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after block body.");
      break;
    }

    if (match(compiler, TOKEN_RIGHT_BRACE)) break;
  }
}

// The VM can only handle a certain number of parameters, so check that we
// haven't exceeded that and give a usable error.
static void validateNumParameters(Compiler* compiler, int numArgs)
{
  if (numArgs == MAX_PARAMETERS + 1)
  {
    // Only show an error at exactly max + 1 so that we can keep parsing the
    // parameters and minimize cascaded errors.
    error(compiler, "Methods cannot have more than %d parameters.",
          MAX_PARAMETERS);
  }
}

// Parses an optional parenthesis-delimited parameter list. If the parameter
// list is for a method, [name] will be the name of the method and this will
// modify it to handle arity in the signature. For functions, [name] will be
// `null`.
static void parameterList(Compiler* compiler, char* name, int* length)
{
  // Parse the parameter list, if any.
  if (match(compiler, TOKEN_LEFT_PAREN))
  {
    int numParams = 0;
    do
    {
      validateNumParameters(compiler, ++numParams);

      // Define a local variable in the method for the parameter.
      declareVariable(compiler);

      // Add a space in the name for the parameter.
      if (name != NULL) name[(*length)++] = ' ';
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  }
}

// Compiles an (optional) argument list and then calls it.
static void methodCall(Compiler* compiler, Code instruction,
                       char name[MAX_METHOD_SIGNATURE], int length)
{
  // Parse the argument list, if any.
  int numArgs = 0;
  if (match(compiler, TOKEN_LEFT_PAREN))
  {
    do
    {
      validateNumParameters(compiler, ++numArgs);
      expression(compiler);

      // Add a space in the name for each argument. Lets us overload by
      // arity.
      name[length++] = ' ';
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  }

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  emit1(compiler, instruction + numArgs, symbol);
}

// Compiles an expression that starts with ".name". That includes getters,
// method calls with arguments, and setter calls.
static void namedCall(Compiler* compiler, bool allowAssignment,
                      Code instruction)
{
  // Build the method name.
  consume(compiler, TOKEN_NAME, "Expect method name after '.'.");
  char name[MAX_METHOD_SIGNATURE];
  int length = copyName(compiler, name);

  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    name[length++] = '=';
    name[length++] = ' ';

    // Compile the assigned value.
    expression(compiler);

    int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);
    emit1(compiler, instruction + 1, symbol);
  }
  else
  {
    methodCall(compiler, instruction, name, length);
  }
}

// Loads the receiver of the currently enclosing method. Correctly handles
// functions defined inside methods.
static void loadThis(Compiler* compiler)
{
  Code loadInstruction;
  int index = resolveName(compiler, "this", 4, &loadInstruction);
  emit1(compiler, loadInstruction, index);
}

static void grouping(Compiler* compiler, bool allowAssignment)
{
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void list(Compiler* compiler, bool allowAssignment)
{
  // Compile the list elements.
  int numElements = 0;
  if (peek(compiler) != TOKEN_RIGHT_BRACKET)
  {
    do
    {
      numElements++;
      expression(compiler);

      // Ignore a newline after the element but before the ',' or ']'.
      match(compiler, TOKEN_LINE);
    } while (match(compiler, TOKEN_COMMA));
  }

  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");

  // Create the list.
  // TODO: Handle lists >255 elements.
  emit1(compiler, CODE_LIST, numElements);
}

// Unary operators like `-foo`.
static void unaryOp(Compiler* compiler, bool allowAssignment)
{
  GrammarRule* rule = &rules[compiler->parser->previous.type];

  // Compile the argument.
  parsePrecedence(compiler, false, PREC_UNARY + 1);

  // Call the operator method on the left-hand side.
  int symbol = ensureSymbol(&compiler->parser->vm->methods, rule->name, 1);
  emit1(compiler, CODE_CALL_0, symbol);
}

static void boolean(Compiler* compiler, bool allowAssignment)
{
  emit(compiler,
       compiler->parser->previous.type == TOKEN_FALSE ? CODE_FALSE : CODE_TRUE);
}

static void function(Compiler* compiler, bool allowAssignment)
{
  Compiler fnCompiler;
  initCompiler(&fnCompiler, compiler->parser, compiler, NULL, 0);

  parameterList(&fnCompiler, NULL, NULL);

  if (match(&fnCompiler, TOKEN_LEFT_BRACE))
  {
    // Block body.
    finishBlock(&fnCompiler);

    // Implicitly return null.
    emit(&fnCompiler, CODE_NULL);
    emit(&fnCompiler, CODE_RETURN);
  }
  else
  {
    // Single expression body.
    expression(&fnCompiler);
    emit(&fnCompiler, CODE_RETURN);
  }

  endCompiler(&fnCompiler);
}

static void field(Compiler* compiler, bool allowAssignment)
{
  int field;
  if (compiler->fields != NULL)
  {
    // Look up the field, or implicitly define it.
    field = ensureSymbol(compiler->fields,
        compiler->parser->previous.start,
        compiler->parser->previous.length);
  }
  else
  {
    error(compiler, "Cannot reference a field outside of a class definition.");
    // Initialize it with a fake value so we can keep parsing and minimize the
    // number of cascaded errors.
    field = 255;
  }

  // If there's an "=" after a field name, it's an assignment.
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    expression(compiler);

    // If we're directly inside a method, use a more optimal instruction.
    if (compiler->methodName != NULL)
    {
      emit1(compiler, CODE_STORE_FIELD_THIS, field);
    }
    else
    {
      loadThis(compiler);
      emit1(compiler, CODE_STORE_FIELD, field);
    }
  }
  else
  {
    // If we're directly inside a method, use a more optimal instruction.
    if (compiler->methodName != NULL)
    {
      emit1(compiler, CODE_LOAD_FIELD_THIS, field);
    }
    else
    {
      loadThis(compiler);
      emit1(compiler, CODE_LOAD_FIELD, field);
    }
  }
}

static void name(Compiler* compiler, bool allowAssignment)
{
  // Look up the name in the scope chain.
  Token* token = &compiler->parser->previous;

  Code loadInstruction;
  int index = resolveName(compiler, token->start, token->length,
                          &loadInstruction);
  if (index == -1) error(compiler, "Undefined variable.");

  // If there's an "=" after a bare name, it's a variable assignment.
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    expression(compiler);

    // Emit the store instruction.
    switch (loadInstruction)
    {
      case CODE_LOAD_LOCAL: emit1(compiler, CODE_STORE_LOCAL, index); break;
      case CODE_LOAD_UPVALUE: emit1(compiler, CODE_STORE_UPVALUE, index); break;
      case CODE_LOAD_GLOBAL: emit1(compiler, CODE_STORE_GLOBAL, index); break;
      default:
        UNREACHABLE();
    }
  }
  else
  {
    emit1(compiler, loadInstruction, index);
  }
}

static void null(Compiler* compiler, bool allowAssignment)
{
  emit(compiler, CODE_NULL);
}

static void number(Compiler* compiler, bool allowAssignment)
{
  Token* token = &compiler->parser->previous;
  char* end;

  double value = strtod(token->start, &end);
  // TODO: Check errno == ERANGE here.
  if (end == token->start)
  {
    error(compiler, "Invalid number literal.");
    value = 0;
  }

  // Define a constant for the literal.
  int constant = addConstant(compiler, NUM_VAL(value));

  // Compile the code to load the constant.
  emit1(compiler, CODE_CONSTANT, constant);
}

static void string(Compiler* compiler, bool allowAssignment)
{
  // Define a constant for the literal.
  int constant = addConstant(compiler, wrenNewString(compiler->parser->vm,
      (char*)compiler->parser->string.bytes, compiler->parser->string.length));

  freeBuffer(compiler->parser->vm, &compiler->parser->string);

  // Compile the code to load the constant.
  emit1(compiler, CODE_CONSTANT, constant);
}

// Returns true if [compiler] is compiling a chunk of code that is either
// directly or indirectly contained in a method for a class.
static bool isInsideMethod(Compiler* compiler)
{
  // Walk up the parent chain to see if there is an enclosing method.
  while (compiler != NULL)
  {
    if (compiler->methodName != NULL) return true;
    compiler = compiler->parent;
  }

  return false;
}

static void new_(Compiler* compiler, bool allowAssignment)
{
  // TODO: Instead of an expression, explicitly only allow a dotted name.
  // Compile the class.
  parsePrecedence(compiler, false, PREC_CALL);

  // Create the instance of the class.
  emit(compiler, CODE_NEW);

  // Invoke the constructor on the new instance.
  char name[MAX_METHOD_SIGNATURE];
  strcpy(name, "new");
  methodCall(compiler, CODE_CALL_0, name, 3);
}

static void super_(Compiler* compiler, bool allowAssignment)
{
  if (!isInsideMethod(compiler))
  {
    error(compiler, "Cannot use 'super' outside of a method.");
  }

  loadThis(compiler);

  // TODO: Super operator calls.

  // See if it's a named super call, or an unnamed one.
  if (match(compiler, TOKEN_DOT))
  {
    // Compile the superclass call.
    namedCall(compiler, allowAssignment, CODE_SUPER_0);
  }
  else
  {
    // No explicit name, so use the name of the enclosing method.
    char name[MAX_METHOD_SIGNATURE];
    int length = 0;
    
    Compiler* thisCompiler = compiler;
    while (thisCompiler != NULL)
    {
      if (thisCompiler->methodName != NULL)
      {
        length = thisCompiler->methodLength;
        strncpy(name, thisCompiler->methodName, length);
        break;
      }

      thisCompiler = thisCompiler->parent;
    }

    // Call the superclass method with the same name.
    methodCall(compiler, CODE_SUPER_0, name, length);
  }
}

static void this_(Compiler* compiler, bool allowAssignment)
{
  if (!isInsideMethod(compiler))
  {
    error(compiler, "Cannot use 'this' outside of a method.");
    return;
  }

  loadThis(compiler);
}

// Subscript or "array indexing" operator like `foo[bar]`.
static void subscript(Compiler* compiler, bool allowAssignment)
{
  char name[MAX_METHOD_SIGNATURE];
  int length = 1;
  int numArgs = 0;

  // Build the method name. To allow overloading by arity, we add a space to
  // the name for each argument.
  name[0] = '[';

  // Parse the argument list.
  do
  {
    validateNumParameters(compiler, ++numArgs);
    expression(compiler);

    // Add a space in the name for each argument. Lets us overload by
    // arity.
    name[length++] = ' ';
  }
  while (match(compiler, TOKEN_COMMA));

  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after arguments.");

  name[length++] = ']';

  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    name[length++] = '=';

    // Compile the assigned value.
    validateNumParameters(compiler, ++numArgs);
    expression(compiler);
  }

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  // Compile the method call.
  emit1(compiler, CODE_CALL_0 + numArgs, symbol);
}

void call(Compiler* compiler, bool allowAssignment)
{
  namedCall(compiler, allowAssignment, CODE_CALL_0);
}

void is(Compiler* compiler, bool allowAssignment)
{
  // Compile the right-hand side.
  parsePrecedence(compiler, false, PREC_CALL);

  emit(compiler, CODE_IS);
}

void and(Compiler* compiler, bool allowAssignment)
{
  // Skip the right argument if the left is false.
  int jump = emit1(compiler, CODE_AND, 255);
  parsePrecedence(compiler, false, PREC_LOGIC);
  patchJump(compiler, jump);
}

void or(Compiler* compiler, bool allowAssignment)
{
  // Skip the right argument if the left is true.
  int jump = emit1(compiler, CODE_OR, 255);
  parsePrecedence(compiler, false, PREC_LOGIC);
  patchJump(compiler, jump);
}

void infixOp(Compiler* compiler, bool allowAssignment)
{
  GrammarRule* rule = &rules[compiler->parser->previous.type];

  // Compile the right-hand side.
  parsePrecedence(compiler, false, rule->precedence + 1);

  // Call the operator method on the left-hand side.
  int symbol = ensureSymbol(&compiler->parser->vm->methods,
                            rule->name, strlen(rule->name));
  emit1(compiler, CODE_CALL_1, symbol);
}

// Compiles a method signature for an infix operator.
void infixSignature(Compiler* compiler, char* name, int* length)
{
  // Add a space for the RHS parameter.
  name[(*length)++] = ' ';

  // Parse the parameter name.
  declareVariable(compiler);
}

// Compiles a method signature for an unary operator (i.e. "!").
void unarySignature(Compiler* compiler, char* name, int* length)
{
  // Do nothing. The name is already complete.
}

// Compiles a method signature for an operator that can either be unary or
// infix (i.e. "-").
void mixedSignature(Compiler* compiler, char* name, int* length)
{
  // If there is a parameter name, it's an infix operator, otherwise it's unary.
  if (compiler->parser->current.type == TOKEN_NAME)
  {
    // Add a space for the RHS parameter.
    name[(*length)++] = ' ';

    // Parse the parameter name.
    declareVariable(compiler);
  }
}

// Compiles a method signature for a named method or setter.
void namedSignature(Compiler* compiler, char* name, int* length)
{
  if (match(compiler, TOKEN_EQ))
  {
    // It's a setter.
    // TODO: Allow setters with parameters? Like: foo.bar(1, 2) = "blah"
    name[(*length)++] = '=';
    name[(*length)++] = ' ';

    // Parse the value parameter.
    declareVariable(compiler);
  }
  else
  {
    // Regular named method with an optional parameter list.
    parameterList(compiler, name, length);
  }
}

// Compiles a method signature for a constructor.
void constructorSignature(Compiler* compiler, char* name, int* length)
{
  // Add the parameters, if there are any.
  parameterList(compiler, name, length);
}

// This table defines all of the parsing rules for the prefix and infix
// expressions in the grammar. Expressions are parsed using a Pratt parser.
//
// See: http://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/
#define UNUSED                     { NULL, NULL, NULL, PREC_NONE, NULL }
#define PREFIX(fn)                 { fn, NULL, NULL, PREC_NONE, NULL }
#define INFIX(prec, fn)            { NULL, fn, NULL, prec, NULL }
#define INFIX_OPERATOR(prec, name) { NULL, infixOp, infixSignature, prec, name }
#define PREFIX_OPERATOR(name)      { unaryOp, NULL, unarySignature, PREC_NONE, name }
#define OPERATOR(name)             { unaryOp, infixOp, mixedSignature, PREC_TERM, name }

GrammarRule rules[] =
{
  /* TOKEN_LEFT_PAREN    */ PREFIX(grouping),
  /* TOKEN_RIGHT_PAREN   */ UNUSED,
  /* TOKEN_LEFT_BRACKET  */ { list, subscript, NULL, PREC_CALL, NULL },
  /* TOKEN_RIGHT_BRACKET */ UNUSED,
  /* TOKEN_LEFT_BRACE    */ UNUSED,
  /* TOKEN_RIGHT_BRACE   */ UNUSED,
  /* TOKEN_COLON         */ UNUSED,
  /* TOKEN_DOT           */ INFIX(PREC_CALL, call),
  /* TOKEN_DOTDOT        */ INFIX_OPERATOR(PREC_RANGE, ".. "),
  /* TOKEN_DOTDOTDOT     */ INFIX_OPERATOR(PREC_RANGE, "... "),
  /* TOKEN_COMMA         */ UNUSED,
  /* TOKEN_STAR          */ INFIX_OPERATOR(PREC_FACTOR, "* "),
  /* TOKEN_SLASH         */ INFIX_OPERATOR(PREC_FACTOR, "/ "),
  /* TOKEN_PERCENT       */ INFIX_OPERATOR(PREC_TERM, "% "),
  /* TOKEN_PLUS          */ INFIX_OPERATOR(PREC_TERM, "+ "),
  /* TOKEN_MINUS         */ OPERATOR("- "),
  /* TOKEN_PIPE          */ UNUSED,
  /* TOKEN_PIPEPIPE      */ INFIX(PREC_LOGIC, or),
  /* TOKEN_AMP           */ UNUSED,
  /* TOKEN_AMPAMP        */ INFIX(PREC_LOGIC, and),
  /* TOKEN_BANG          */ PREFIX_OPERATOR("!"),
  /* TOKEN_TILDE         */ PREFIX_OPERATOR("~"),
  /* TOKEN_EQ            */ UNUSED,
  /* TOKEN_LT            */ INFIX_OPERATOR(PREC_COMPARISON, "< "),
  /* TOKEN_GT            */ INFIX_OPERATOR(PREC_COMPARISON, "> "),
  /* TOKEN_LTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, "<= "),
  /* TOKEN_GTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, ">= "),
  /* TOKEN_EQEQ          */ INFIX_OPERATOR(PREC_EQUALITY, "== "),
  /* TOKEN_BANGEQ        */ INFIX_OPERATOR(PREC_EQUALITY, "!= "),
  /* TOKEN_BREAK         */ UNUSED,
  /* TOKEN_CLASS         */ UNUSED,
  /* TOKEN_ELSE          */ UNUSED,
  /* TOKEN_FALSE         */ PREFIX(boolean),
  /* TOKEN_FN            */ PREFIX(function),
  /* TOKEN_FOR           */ UNUSED,
  /* TOKEN_IF            */ UNUSED,
  /* TOKEN_IN            */ UNUSED,
  /* TOKEN_IS            */ INFIX(PREC_IS, is),
  /* TOKEN_NEW           */ { new_, NULL, constructorSignature, PREC_NONE, NULL },
  /* TOKEN_NULL          */ PREFIX(null),
  /* TOKEN_RETURN        */ UNUSED,
  /* TOKEN_STATIC        */ UNUSED,
  /* TOKEN_SUPER         */ PREFIX(super_),
  /* TOKEN_THIS          */ PREFIX(this_),
  /* TOKEN_TRUE          */ PREFIX(boolean),
  /* TOKEN_VAR           */ UNUSED,
  /* TOKEN_WHILE         */ UNUSED,
  /* TOKEN_FIELD         */ PREFIX(field),
  /* TOKEN_NAME          */ { name, NULL, namedSignature, PREC_NONE, NULL },
  /* TOKEN_NUMBER        */ PREFIX(number),
  /* TOKEN_STRING        */ PREFIX(string),
  /* TOKEN_LINE          */ UNUSED,
  /* TOKEN_ERROR         */ UNUSED,
  /* TOKEN_EOF           */ UNUSED
};

// The main entrypoint for the top-down operator precedence parser.
void parsePrecedence(Compiler* compiler, bool allowAssignment,
                     Precedence precedence)
{
  nextToken(compiler->parser);
  GrammarFn prefix = rules[compiler->parser->previous.type].prefix;

  if (prefix == NULL)
  {
    error(compiler, "Unexpected token for expression.");
    return;
  }

  prefix(compiler, allowAssignment);

  while (precedence <= rules[compiler->parser->current.type].precedence)
  {
    nextToken(compiler->parser);
    GrammarFn infix = rules[compiler->parser->previous.type].infix;
    infix(compiler, allowAssignment);
  }
}

// Parses an expression. Unlike statements, expressions leave a resulting value
// on the stack.
void expression(Compiler* compiler)
{
  parsePrecedence(compiler, true, PREC_LOWEST);
}

// Compiles a method definition inside a class body.
void method(Compiler* compiler, Code instruction, bool isConstructor,
            SignatureFn signature)
{
  // Build the method name.
  char name[MAX_METHOD_SIGNATURE];
  int length = copyName(compiler, name);

  Compiler methodCompiler;
  initCompiler(&methodCompiler, compiler->parser, compiler, name, length);

  // Compile the method signature.
  signature(&methodCompiler, name, &length);

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  consume(compiler, TOKEN_LEFT_BRACE, "Expect '{' to begin method body.");

  finishBlock(&methodCompiler);
  // TODO: Single-expression methods that implicitly return the result.

  // If it's a constructor, return "this".
  if (isConstructor)
  {
    // The receiver is always stored in the first local slot.
    emit1(&methodCompiler, CODE_LOAD_LOCAL, 0);
  }
  else
  {
    // Implicitly return null in case there is no explicit return.
    emit(&methodCompiler, CODE_NULL);
  }

  emit(&methodCompiler, CODE_RETURN);

  endCompiler(&methodCompiler);

  // Compile the code to define the method.
  emit1(compiler, instruction, symbol);
}

// Parses a curly block or an expression statement. Used in places like the
// arms of an if statement where either a single expression or a curly body is
// allowed.
void block(Compiler* compiler)
{
  // Curly block.
  if (match(compiler, TOKEN_LEFT_BRACE))
  {
    pushScope(compiler);
    finishBlock(compiler);
    popScope(compiler);
    return;
  }

  // Single statement body.
  statement(compiler);
}

// Returns the number of arguments to the instruction at [ip] in [fn]'s
// bytecode.
static int getNumArguments(const uint8_t* bytecode, const Value* constants,
                           int ip)
{
  Code instruction = bytecode[ip];
  switch (instruction)
  {
    case CODE_NULL:
    case CODE_FALSE:
    case CODE_TRUE:
    case CODE_POP:
    case CODE_IS:
    case CODE_CLOSE_UPVALUE:
    case CODE_RETURN:
    case CODE_NEW:
      return 0;

      // Instructions with two arguments:
    case CODE_METHOD_INSTANCE:
    case CODE_METHOD_STATIC:
      return 2;

    case CODE_CLOSURE:
    {
      int constant = bytecode[ip + 1];
      ObjFn* loadedFn = AS_FN(constants[constant]);

      // There is an argument for the constant, then one for each upvalue.
      return 1 + loadedFn->numUpvalues;
    }

    default:
      // Most instructions have one argument.
      return 1;
  }
}

static int startLoopBody(Compiler* compiler)
{
  int outerLoopBody = compiler->loopBody;
  compiler->loopBody = compiler->bytecode.length;
  return outerLoopBody;
}

static void endLoopBody(Compiler* compiler, int outerLoopBody)
{
  // Find any break placeholder instructions (which will be CODE_END in the
  // bytecode) and replace them with real jumps.
  int i = compiler->loopBody;
  while (i < compiler->bytecode.length)
  {
    if (compiler->bytecode.bytes[i] == CODE_END)
    {
      compiler->bytecode.bytes[i] = CODE_JUMP;
      patchJump(compiler, i + 1);
      i += 2;
    }
    else
    {
      // Skip this instruction and its arguments.
      i += 1 + getNumArguments(compiler->bytecode.bytes,
                               compiler->constants->elements, i);
    }
  }

  compiler->loopBody = outerLoopBody;
}

static void forStatement(Compiler* compiler)
{
  // A for statement like:
  //
  //     for (i in sequence.expression) {
  //       IO.write(i)
  //     }
  //
  // Is compiled to bytecode almost as if the source looked like this:
  //
  //     {
  //       var seq_ = sequence.expression
  //       var iter_
  //       while (true) {
  //         iter_ = seq_.iterate(iter_)
  //         if (!iter_) break
  //         var i = set_.iteratorValue(iter_)
  //         IO.write(i)
  //       }
  //     }
  //
  // It's not exactly this, because the synthetic variables `seq_` and `iter_`
  // actually get names that aren't valid Wren identfiers. Also, the `while`
  // and `break` are just the bytecode for explicit loops and jumps. But that's
  // the basic idea.
  //
  // The important parts are:
  // - The sequence expression is only evaluated once.
  // - The .iterate() method is used to advance the iterator and determine if
  //   it should exit the loop.
  // - The .iteratorValue() method is used to get the value at the current
  //   iterator position.

  // Create a scope for the hidden local variables used for the iterator.
  pushScope(compiler);

  consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
  consume(compiler, TOKEN_NAME, "Expect for loop variable name.");

  // Remember the name of the loop variable.
  const char* name = compiler->parser->previous.start;
  int length = compiler->parser->previous.length;

  consume(compiler, TOKEN_IN, "Expect 'in' after loop variable.");

  // Evaluate the sequence expression and store it in a hidden local variable.
  // The space in the variable name ensures it won't collide with a user-defined
  // variable.
  expression(compiler);
  int seqSlot = defineLocal(compiler, "seq ", 4);

  // Create another hidden local for the iterator object.
  null(compiler, false);
  int iterSlot = defineLocal(compiler, "iter ", 5);

  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after loop expression.");

  // Remember what instruction to loop back to.
  int loopStart = compiler->bytecode.length - 1;

  // Advance the iterator by calling the ".iterate" method on the sequence.
  emit1(compiler, CODE_LOAD_LOCAL, seqSlot);
  emit1(compiler, CODE_LOAD_LOCAL, iterSlot);

  int iterateSymbol = ensureSymbol(&compiler->parser->vm->methods,
                                   "iterate ", 8);
  emit1(compiler, CODE_CALL_1, iterateSymbol);

  // Store the iterator back in its local for the next iteration.
  emit1(compiler, CODE_STORE_LOCAL, iterSlot);
  // TODO: We can probably get this working with a bit less stack juggling.

  // If it returned something falsy, jump out of the loop.
  int exitJump = emit1(compiler, CODE_JUMP_IF, 255);

  // Create a scope for the loop body.
  pushScope(compiler);

  // Get the current value in the sequence by calling ".iteratorValue".
  emit1(compiler, CODE_LOAD_LOCAL, seqSlot);
  emit1(compiler, CODE_LOAD_LOCAL, iterSlot);

  int iteratorValueSymbol = ensureSymbol(&compiler->parser->vm->methods,
                                         "iteratorValue ", 14);
  emit1(compiler, CODE_CALL_1, iteratorValueSymbol);

  // Bind it to the loop variable.
  defineLocal(compiler, name, length);

  // Compile the body.
  int outerLoopBody = startLoopBody(compiler);
  block(compiler);

  popScope(compiler);

  // Loop back to the top.
  emit(compiler, CODE_LOOP);
  int loopOffset = compiler->bytecode.length - loopStart;
  emit(compiler, loopOffset);

  patchJump(compiler, exitJump);
  endLoopBody(compiler, outerLoopBody);

  popScope(compiler);
}

static void whileStatement(Compiler* compiler)
{
  // Remember what instruction to loop back to.
  int loopStart = compiler->bytecode.length - 1;

  // Compile the condition.
  consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

  int exitJump = emit1(compiler, CODE_JUMP_IF, 255);

  // Compile the body.
  int outerLoopBody = startLoopBody(compiler);
  block(compiler);

  // Loop back to the top.
  emit(compiler, CODE_LOOP);
  int loopOffset = compiler->bytecode.length - loopStart;
  emit(compiler, loopOffset);

  patchJump(compiler, exitJump);
  endLoopBody(compiler, outerLoopBody);
}

// Compiles a statement. These can only appear at the top-level or within
// curly blocks. Unlike expressions, these do not leave a value on the stack.
void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_BREAK))
  {
    if (compiler->loopBody == -1)
    {
      error(compiler, "Cannot use 'break' outside of a loop.");
    }

    // Emit a placeholder instruction for the jump to the end of the body. When
    // we're done compiling the loop body and know where the end is, we'll
    // replace these with `CODE_JUMP` instructions with appropriate offsets.
    // We use `CODE_END` here because that can't occur in the middle of
    // bytecode.
    emit1(compiler, CODE_END, 0);
    return;
  }

  if (match(compiler, TOKEN_FOR)) return forStatement(compiler);

  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");

    // Jump to the else branch if the condition is false.
    int ifJump = emit1(compiler, CODE_JUMP_IF, 255);

    // Compile the then branch.
    block(compiler);

    // Compile the else branch if there is one.
    if (match(compiler, TOKEN_ELSE))
    {
      // Jump over the else branch when the if branch is taken.
      int elseJump = emit1(compiler, CODE_JUMP, 255);

      patchJump(compiler, ifJump);
      
      block(compiler);

      // Patch the jump over the else.
      patchJump(compiler, elseJump);
    }
    else
    {
      patchJump(compiler, ifJump);
    }

    return;
  }

  if (match(compiler, TOKEN_RETURN))
  {
    // Compile the return value.
    // TODO: Implicitly return null if there is a newline or } after the
    // "return".
    expression(compiler);

    emit(compiler, CODE_RETURN);
    return;
  }

  if (match(compiler, TOKEN_WHILE)) return whileStatement(compiler);

  // Expression statement.
  expression(compiler);
  emit(compiler, CODE_POP);
}

// Compiles a class definition. Assumes the "class" token has already been
// consumed.
static void classDefinition(Compiler* compiler)
{
  // Create a variable to store the class in.
  // TODO: Allow anonymous classes?
  int symbol = declareVariable(compiler);

  // Load the superclass (if there is one).
  if (match(compiler, TOKEN_IS))
  {
    parsePrecedence(compiler, false, PREC_CALL);
    emit(compiler, CODE_SUBCLASS);
  }
  else
  {
    // Create the empty class.
    emit(compiler, CODE_CLASS);
  }

  // Store a placeholder for the number of fields argument. We don't know
  // the value until we've compiled all the methods to see which fields are
  // used.
  int numFieldsInstruction = emit(compiler, 255);

  // Set up a symbol table for the class's fields. We'll initially compile
  // them to slots starting at zero. When the method is bound to the close
  // the bytecode will be adjusted by [wrenBindMethod] to take inherited
  // fields into account.
  SymbolTable* previousFields = compiler->fields;
  SymbolTable fields;
  initSymbolTable(&fields);
  compiler->fields = &fields;

  // Compile the method definitions.
  consume(compiler, TOKEN_LEFT_BRACE, "Expect '}' after class body.");
  while (!match(compiler, TOKEN_RIGHT_BRACE))
  {
    Code instruction = CODE_METHOD_INSTANCE;
    bool isConstructor = false;

    if (match(compiler, TOKEN_STATIC))
    {
      instruction = CODE_METHOD_STATIC;
      // TODO: Need to handle fields inside static methods correctly.
      // Currently, they're compiled as instance fields, which will be wrong
      // wrong wrong given that the receiver is actually the class obj.
    }
    else if (peek(compiler) == TOKEN_NEW)
    {
      // If the method name is "new", it's a constructor.
      isConstructor = true;
    }

    SignatureFn signature = rules[compiler->parser->current.type].method;
    nextToken(compiler->parser);

    if (signature == NULL)
    {
      error(compiler, "Expect method definition.");
      break;
    }

    method(compiler, instruction, isConstructor, signature);
    consume(compiler, TOKEN_LINE,
            "Expect newline after definition in class.");
  }

  // Update the class with the number of fields.
  compiler->bytecode.bytes[numFieldsInstruction] = fields.count;
  compiler->fields = previousFields;

  // Store it in its name.
  defineVariable(compiler, symbol);
}

static void variableDefinition(Compiler* compiler)
{
  // TODO: Variable should not be in scope until after initializer.
  int symbol = declareVariable(compiler);

  // Compile the initializer.
  if (match(compiler, TOKEN_EQ))
  {
    expression(compiler);
  }
  else
  {
    // Default initialize it to null.
    null(compiler, false);
  }

  defineVariable(compiler, symbol);
}

// Compiles a "definition". These are the statements that bind new variables.
// They can only appear at the top level of a block and are prohibited in places
// like the non-curly body of an if or while.
void definition(Compiler* compiler)
{
  if (match(compiler, TOKEN_CLASS)) return classDefinition(compiler);
  if (match(compiler, TOKEN_VAR)) return variableDefinition(compiler);

  block(compiler);
}

// Parses [source] to a "function" (a chunk of top-level code) for execution by
// [vm].
ObjFn* wrenCompile(WrenVM* vm, const char* source)
{
  Parser parser;
  parser.vm = vm;
  parser.source = source;
  parser.hasError = false;

  // Ignore leading newlines.
  parser.skipNewlines = true;

  parser.tokenStart = source;
  parser.currentChar = source;
  parser.currentLine = 1;

  // Zero-init the current token. This will get copied to previous when
  // advance() is called below.
  parser.current.type = TOKEN_ERROR;
  parser.current.start = source;
  parser.current.length = 0;
  parser.current.line = 0;

  initBuffer(vm, &parser.string);

  // Read the first token.
  nextToken(&parser);

  Compiler compiler;
  initCompiler(&compiler, &parser, NULL, NULL, 0);

  for (;;)
  {
    definition(&compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(&compiler, TOKEN_LINE))
    {
      consume(&compiler, TOKEN_EOF, "Expect end of file.");
      break;
    }

    if (match(&compiler, TOKEN_EOF)) break;
  }

  emit(&compiler, CODE_NULL);
  emit(&compiler, CODE_RETURN);

  return endCompiler(&compiler);
}

void wrenBindMethod(ObjClass* classObj, ObjFn* fn)
{
  // TODO: What about functions nested inside [fn]?
  int ip = 0;
  for (;;)
  {
    Code instruction = fn->bytecode[ip++];
    switch (instruction)
    {
      case CODE_LOAD_FIELD:
      case CODE_STORE_FIELD:
      case CODE_LOAD_FIELD_THIS:
      case CODE_STORE_FIELD_THIS:
        // Shift this class's fields down past the inherited ones.
        fn->bytecode[ip++] += classObj->superclass->numFields;
        break;

      case CODE_END:
        return;

      default:
        // Other instructions are unaffected, so just skip over them.
        ip += getNumArguments(fn->bytecode, fn->constants, ip - 1);
        break;
    }
  }
}

void wrenMarkCompiler(WrenVM* vm, Compiler* compiler)
{
  // Walk up the parent chain to mark the outer compilers too. The VM only
  // tracks the innermost one.
  while (compiler != NULL)
  {
    if (compiler->constants != NULL)
    {
      wrenMarkValue(vm, OBJ_VAL(compiler->constants));
    }

    compiler = compiler->parent;
  }
}
