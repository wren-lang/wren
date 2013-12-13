#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "wren_common.h"
#include "wren_compiler.h"

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

// The maximum name of a method, not including the signature. This is an
// arbitrary but enforced maximum just so we know how long the method name
// strings need to be in the parser.
#define MAX_METHOD_NAME (64)

// The maximum length of a method signature. This includes the name, and the
// extra spaces added to handle arity, and another byte to terminate the string.
#define MAX_METHOD_SIGNATURE (MAX_METHOD_NAME + MAX_PARAMETERS + 1)

// TODO(bob): Get rid of this and use a growable buffer.
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

  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FN,
  TOKEN_IF,
  TOKEN_IS,
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

  // Non-zero if subsequent newline tokens should be discarded.
  int skipNewlines;

  // Non-zero if a syntax or compile error has occurred.
  int hasError;

  // TODO(bob): Dynamically allocate this.
  // A buffer for the unescaped text of the current token if it's a string
  // literal. Unlike the raw token, this will have escape sequences translated
  // to their literal equivalent.
  char currentString[MAX_STRING];
  int currentStringLength;
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

  // Non-zero if this local variable is being used as an upvalue.
  int isUpvalue;
} Local;

typedef struct
{
  // Non-zero if this upvalue is capturing a local variable from the enclosing
  // function. Zero if it's capturing an upvalue.
  int isLocal;

  // The index of the local or upvalue being captured in the enclosing function.
  int index;
} CompilerUpvalue;

typedef struct sCompiler
{
  Parser* parser;

  // The compiler for the function enclosing this one, or NULL if it's the
  // top level.
  struct sCompiler* parent;

  // The function being compiled.
  ObjFn* fn;
  int numCodes;

  // Symbol table for the fields of the nearest enclosing class, or NULL if not
  // currently inside a class.
  SymbolTable* fields;

  // Non-zero if the function being compiled is a method.
  int isMethod;

  // The number of local variables currently in scope.
  int numLocals;

  // The current level of block scope nesting, where zero is no nesting. A -1
  // here means top-level code is being compiled and there is no block scope
  // in effect at all. Any variables declared will be global.
  int scopeDepth;

  // The currently in scope local variables.
  Local locals[MAX_LOCALS];

  // The upvalues that this function has captured from outer scopes. The count
  // of them is stored in `fn->numUpvalues`.
  CompilerUpvalue upvalues[MAX_UPVALUES];
} Compiler;

// Adds [constant] to the constant pool and returns its index.
static int addConstant(Compiler* compiler, Value constant)
{
  // TODO(bob): Look for existing equal constant. Note that we need to *not*
  // do that for the placeholder constant created for super calls.
  // TODO(bob): Check for overflow.
  compiler->fn->constants[compiler->fn->numConstants++] = constant;
  return compiler->fn->numConstants - 1;
}

// Initializes [compiler].
static int initCompiler(Compiler* compiler, Parser* parser,
                         Compiler* parent, int isMethod)
{
  compiler->parser = parser;
  compiler->parent = parent;
  compiler->numCodes = 0;
  compiler->isMethod = isMethod;

  if (parent == NULL)
  {
    // Compiling top-level code, so the initial scope is global.
    compiler->scopeDepth = -1;
    compiler->numLocals = 0;
  }
  else
  {
    // Declare a fake local variable for "this" so that it's slot in the stack
    // is taken.
    compiler->numLocals = 1;
    compiler->locals[0].name = NULL;
    compiler->locals[0].length = 0;
    compiler->locals[0].depth = -1;
    compiler->locals[0].isUpvalue = 0;

    // The initial scope for function or method is a local scope.
    compiler->scopeDepth = 0;
  }

  // Propagate the enclosing class downwards.
  compiler->fields = parent != NULL ? parent->fields :  NULL;

  compiler->fn = wrenNewFunction(parser->vm);

  if (parent == NULL) return -1;

  // Add the block to the constant table. Do this eagerly so it's reachable by
  // the GC.
  return addConstant(parent, OBJ_VAL(compiler->fn));
}

// Outputs a compile or syntax error.
static void error(Compiler* compiler, const char* format, ...)
{
  compiler->parser->hasError = 1;

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

// Lexing ----------------------------------------------------------------------

// Returns non-zero if [c] is a valid (non-initial) identifier character.
static int isName(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// Returns non-zero if [c] is a digit.
static int isDigit(char c)
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
    // TODO(bob): Unterminated comment. Should return error.
    if (peekChar(parser) == '\0') return;

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

// Returns non-zero if the current token's text matches [keyword].
static int isKeyword(Parser* parser, const char* keyword)
{
  size_t length = parser->currentChar - parser->tokenStart;
  size_t keywordLength = strlen(keyword);
  return length == keywordLength &&
      strncmp(parser->tokenStart, keyword, length) == 0;
}

// Finishes lexing a number literal.
static void readNumber(Parser* parser)
{
  // TODO(bob): Hex, scientific, etc.
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

  if (isKeyword(parser, "class")) type = TOKEN_CLASS;
  if (isKeyword(parser, "else")) type = TOKEN_ELSE;
  if (isKeyword(parser, "false")) type = TOKEN_FALSE;
  if (isKeyword(parser, "fn")) type = TOKEN_FN;
  if (isKeyword(parser, "if")) type = TOKEN_IF;
  if (isKeyword(parser, "is")) type = TOKEN_IS;
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
  parser->currentString[parser->currentStringLength++] = c;
}

// Finishes lexing a string literal.
static void readString(Parser* parser)
{
  parser->currentStringLength = 0;
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
          // TODO(bob): Emit error token.
          break;
      }

      // TODO(bob): Other escapes (\r, etc.), Unicode escape sequences.
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
      case '.': makeToken(parser, TOKEN_DOT); return;
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
        parser->currentLine++;
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
      case TOKEN_IF:
      case TOKEN_IS:
      case TOKEN_STATIC:
      case TOKEN_SUPER:
      case TOKEN_VAR:
      case TOKEN_WHILE:
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

// Parsing ---------------------------------------------------------------------

// Returns the type of the current token.
static TokenType peek(Compiler* compiler)
{
  return compiler->parser->current.type;
}

// Consumes the current token if its type is [expected]. Returns non-zero if a
// token was consumed.
static int match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return 0;

  nextToken(compiler->parser);
  return 1;
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

// Emits one bytecode instruction or argument.
static int emit(Compiler* compiler, Code code)
{
  compiler->fn->bytecode[compiler->numCodes++] = code;
  return compiler->numCodes - 1;
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

  // Define a new local variable in the current scope.
  Local* local = &compiler->locals[compiler->numLocals];
  local->name = token->start;
  local->length = token->length;
  local->depth = compiler->scopeDepth;
  local->isUpvalue = 0;
  return compiler->numLocals++;
}

// Stores a variable with the previously defined symbol in the current scope.
static void defineVariable(Compiler* compiler, int symbol)
{
  // Store the variable. If it's a local, the result of the initializer is
  // in the correct slot on the stack already so we're done.
  if (compiler->scopeDepth >= 0) return;

  // It's a global variable, so store the value in the global slot and then
  // discard the temporary for the initializer.
  emit(compiler, CODE_STORE_GLOBAL);
  emit(compiler, symbol);
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

// Attempts to look up the previously consumed name token in the local variables
// of [compiler]. If found, returns its index, otherwise returns -1.
static int resolveLocal(Compiler* compiler)
{
  Token* token = &compiler->parser->previous;

  // Look it up in the local scopes. Look in reverse order so that the most
  // nested variable is found first and shadows outer ones.
  for (int i = compiler->numLocals - 1; i >= 0; i--)
  {
    if (compiler->locals[i].length == token->length &&
        strncmp(token->start, compiler->locals[i].name, token->length) == 0)
    {
      return i;
    }
  }

  return -1;
}

// Adds an upvalue to [compiler]'s function with the given properties. Does not
// add one if an upvalue for that variable is already in the list. Returns the
// index of the uvpalue.
static int addUpvalue(Compiler* compiler, int isLocal, int index)
{
  // Look for an existing one.
  for (int i = 0; i < compiler->fn->numUpvalues; i++)
  {
    CompilerUpvalue* upvalue = &compiler->upvalues[i];
    if (upvalue->index == index && upvalue->isLocal == isLocal) return i;
  }

  // If we got here, it's a new upvalue.
  compiler->upvalues[compiler->fn->numUpvalues].isLocal = isLocal;
  compiler->upvalues[compiler->fn->numUpvalues].index = index;
  return compiler->fn->numUpvalues++;
}

// Attempts to look up the previously consumed name token in the functions
// enclosing the one being compiled by [compiler]. If found, it adds an upvalue
// for it to this compiler's list of upvalues (unless it's already in there)
// and returns its index. If not found, returns -1.
//
// If the name is found outside of the immediately enclosing function, this
// will flatten the closure and add upvalues to all of the intermediate
// functions so that it gets walked down to this one.
static int findUpvalue(Compiler* compiler)
{
  // If we are out of enclosing functions, it can't be an upvalue.
  if (compiler->parent == NULL)
  {
    return -1;
  }

  // See if it's a local variable in the immediately enclosing function.
  int local = resolveLocal(compiler->parent);
  if (local != -1)
  {
    // Mark the local as an upvalue so we know to close it when it goes out of
    // scope.
    compiler->parent->locals[local].isUpvalue = 1;

    return addUpvalue(compiler, 1, local);
  }

  // See if it's an upvalue in the immediately enclosing function. In other
  // words, if its a local variable in a non-immediately enclosing function.
  // This will "flatten" closures automatically: it will add upvalues to all
  // of the intermediate functions to get from the function where a local is
  // declared all the way into the possibly deeply nested function that is
  // closing over it.
  int upvalue = findUpvalue(compiler->parent);
  if (upvalue != -1)
  {
    return addUpvalue(compiler, 0, upvalue);
  }

  // If we got here, we walked all the way up the parent chain and couldn't
  // find it.
  return -1;
}

// A name may resolve to refer to a variable in a few different places: local
// scope in the current function, an upvalue for variables being closed over
// from enclosing functions, or a top-level global variable.
typedef enum
{
  NAME_LOCAL,
  NAME_UPVALUE,
  NAME_GLOBAL
} ResolvedName;

// Look up the previously consumed token, which is presumed to be a TOKEN_NAME
// in the current scope to see what name it is bound to. Returns the index of
// the name either in global or local scope. Returns -1 if not found. Sets
// [isGlobal] to non-zero if the name is in global scope, or 0 if in local.
static int resolveName(Compiler* compiler, ResolvedName* resolved)
{
  Token* token = &compiler->parser->previous;

  // Look it up in the local scopes. Look in reverse order so that the most
  // nested variable is found first and shadows outer ones.
  *resolved = NAME_LOCAL;
  int local = resolveLocal(compiler);
  if (local != -1) return local;

  // If we got here, it's not a local, so lets see if we are closing over an
  // outer local.
  *resolved = NAME_UPVALUE;
  int upvalue = findUpvalue(compiler);
  if (upvalue != -1) return upvalue;

  // If we got here, it wasn't in a local scope, so try the global scope.
  *resolved = NAME_GLOBAL;
  return findSymbol(&compiler->parser->vm->globalSymbols,
                    token->start, token->length);
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
static void endCompiler(Compiler* compiler, int constant)
{
  // Mark the end of the bytecode. Since it may contain multiple early returns,
  // we can't rely on CODE_RETURN to tell us we're at the end.
  emit(compiler, CODE_END);

  // In the function that contains this one, load the resulting function object.
  if (compiler->parent != NULL)
  {
    // If the function has no upvalues, we don't need to create a closure.
    // We can just load and run the function directly.
    if (compiler->fn->numUpvalues == 0)
    {
      emit(compiler->parent, CODE_CONSTANT);
      emit(compiler->parent, constant);
    }
    else
    {
      // Capture the upvalues in the new closure object.
      emit(compiler->parent, CODE_CLOSURE);
      emit(compiler->parent, constant);

      // Emit arguments for each upvalue to know whether to capture a local or
      // an upvalue.
      // TODO(bob): Do something more efficient here?
      for (int i = 0; i < compiler->fn->numUpvalues; i++)
      {
        emit(compiler->parent, compiler->upvalues[i].isLocal);
        emit(compiler->parent, compiler->upvalues[i].index);
      }
    }
  }
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
  PREC_BITWISE,    // | &
  PREC_TERM,       // + -
  PREC_FACTOR,     // * / %
  PREC_UNARY,      // unary - ! ~
  PREC_CALL        // . () []
} Precedence;

// Forward declarations since the grammar is recursive.
static void expression(Compiler* compiler);
static void statement(Compiler* compiler);
static void parsePrecedence(Compiler* compiler, int allowAssignment,
                            Precedence precedence);

typedef void (*GrammarFn)(Compiler*, int allowAssignment);

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
  compiler->fn->bytecode[offset] = compiler->numCodes - offset - 1;
}

// Parses a block body, after the initial "{" has been consumed.
static void finishBlock(Compiler* compiler)
{
  for (;;)
  {
    statement(compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(compiler, TOKEN_LINE))
    {
      consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after block body.");
      break;
    }

    if (match(compiler, TOKEN_RIGHT_BRACE)) break;
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
      // The VM can only handle a certain number of parameters, so check for
      // this explicitly and give a usable error.
      if (++numParams == MAX_PARAMETERS + 1)
      {
        // Only show an error at exactly max + 1 and don't break so that we can
        // keep parsing the parameter list and minimize cascaded errors.
        error(compiler, "Cannot have more than %d parameters.",
              MAX_PARAMETERS);
      }

      // Define a local variable in the method for the parameter.
      declareVariable(compiler);

      // Add a space in the name for the parameter.
      if (name != NULL) name[(*length)++] = ' ';
      // TODO(bob): Check for length overflow.
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  }
}

// Compiles the method name and argument list for a "<...>.name(...)" call.
static void namedCall(Compiler* compiler, int allowAssignment, Code instruction)
{
  // Build the method name.
  consume(compiler, TOKEN_NAME, "Expect method name after '.'.");
  char name[MAX_METHOD_SIGNATURE];
  int length = copyName(compiler, name);

  // TODO(bob): Check for "=" here and set assignment and return.

  // Parse the argument list, if any.
  int numArgs = 0;
  if (match(compiler, TOKEN_LEFT_PAREN))
  {
    do
    {
      // The VM can only handle a certain number of parameters, so check for
      // this explicitly and give a usable error.
      if (++numArgs == MAX_PARAMETERS + 1)
      {
        // Only show an error at exactly max + 1 and don't break so that we can
        // keep parsing the parameter list and minimize cascaded errors.
        error(compiler, "Cannot pass more than %d arguments to a method.",
              MAX_PARAMETERS);
      }

      expression(compiler);

      // Add a space in the name for each argument. Lets us overload by
      // arity.
      name[length++] = ' ';
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  }

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  emit(compiler, instruction + numArgs);
  emit(compiler, symbol);
}

static void grouping(Compiler* compiler, int allowAssignment)
{
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void list(Compiler* compiler, int allowAssignment)
{
  // Compile the list elements.
  int numElements = 0;
  if (peek(compiler) != TOKEN_RIGHT_BRACKET)
  {
    do
    {
      numElements++;
      expression(compiler);
    } while (match(compiler, TOKEN_COMMA));
  }

  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");

  // Create the list.
  emit(compiler, CODE_LIST);
  // TODO(bob): Handle lists >255 elements.
  emit(compiler, numElements);
}

// Unary operators like `-foo`.
static void unaryOp(Compiler* compiler, int allowAssignment)
{
  GrammarRule* rule = &rules[compiler->parser->previous.type];

  // Compile the argument.
  parsePrecedence(compiler, 0, PREC_UNARY + 1);

  // Call the operator method on the left-hand side.
  int symbol = ensureSymbol(&compiler->parser->vm->methods, rule->name, 1);
  emit(compiler, CODE_CALL_0);
  emit(compiler, symbol);
}

static void boolean(Compiler* compiler, int allowAssignment)
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

static void function(Compiler* compiler, int allowAssignment)
{
  Compiler fnCompiler;
  int constant = initCompiler(&fnCompiler, compiler->parser, compiler, 0);

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

  endCompiler(&fnCompiler, constant);
}

static void field(Compiler* compiler, int allowAssignment)
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

    emit(compiler, CODE_STORE_FIELD);
    emit(compiler, field);
    return;
  }

  emit(compiler, CODE_LOAD_FIELD);
  emit(compiler, field);
}

static void name(Compiler* compiler, int allowAssignment)
{
  // Look up the name in the scope chain.
  ResolvedName resolved;
  int index = resolveName(compiler, &resolved);
  if (index == -1) error(compiler, "Undefined variable.");

  // If there's an "=" after a bare name, it's a variable assignment.
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    expression(compiler);

    switch (resolved)
    {
      case NAME_LOCAL: emit(compiler, CODE_STORE_LOCAL); break;
      case NAME_UPVALUE: emit(compiler, CODE_STORE_UPVALUE); break;
      case NAME_GLOBAL: emit(compiler, CODE_STORE_GLOBAL); break;
    }

    emit(compiler, index);
    return;
  }

  // Otherwise, it's just a variable access.
  switch (resolved)
  {
    case NAME_LOCAL: emit(compiler, CODE_LOAD_LOCAL); break;
    case NAME_UPVALUE: emit(compiler, CODE_LOAD_UPVALUE); break;
    case NAME_GLOBAL: emit(compiler, CODE_LOAD_GLOBAL); break;
  }

  emit(compiler, index);
}

static void null(Compiler* compiler, int allowAssignment)
{
  emit(compiler, CODE_NULL);
}

static void number(Compiler* compiler, int allowAssignment)
{
  Token* token = &compiler->parser->previous;
  char* end;

  double value = strtod(token->start, &end);
  // TODO(bob): Check errno == ERANGE here.
  if (end == token->start)
  {
    error(compiler, "Invalid number literal.");
    value = 0;
  }

  // Define a constant for the literal.
  int constant = addConstant(compiler, NUM_VAL(value));

  // Compile the code to load the constant.
  emit(compiler, CODE_CONSTANT);
  emit(compiler, constant);
}

static void string(Compiler* compiler, int allowAssignment)
{
  // Define a constant for the literal.
  int constant = addConstant(compiler, wrenNewString(compiler->parser->vm,
      compiler->parser->currentString, compiler->parser->currentStringLength));

  // Compile the code to load the constant.
  emit(compiler, CODE_CONSTANT);
  emit(compiler, constant);
}

static void super_(Compiler* compiler, int allowAssignment)
{
  // TODO(bob): Error if this is not in a method.
  // The receiver is always stored in the first local slot.
  // TODO(bob): Will need to do something different to handle functions
  // enclosed in methods.
  emit(compiler, CODE_LOAD_LOCAL);
  emit(compiler, 0);

  // TODO(bob): Super operator and constructor calls.
  consume(compiler, TOKEN_DOT, "Expect '.' after 'super'.");

  // Compile the superclass call.
  namedCall(compiler, allowAssignment, CODE_SUPER_0);
}

static void this_(Compiler* compiler, int allowAssignment)
{
  // Walk up the parent chain to see if there is an enclosing method.
  Compiler* thisCompiler = compiler;
  int insideMethod = 0;
  while (thisCompiler != NULL)
  {
    if (thisCompiler->isMethod)
    {
      insideMethod = 1;
      break;
    }

    thisCompiler = thisCompiler->parent;
  }

  if (!insideMethod)
  {
    error(compiler, "Cannot use 'this' outside of a method.");
    return;
  }

  // The receiver is always stored in the first local slot.
  // TODO(bob): Will need to do something different to handle functions
  // enclosed in methods.
  emit(compiler, CODE_LOAD_LOCAL);
  emit(compiler, 0);
}

// Subscript or "array indexing" operator like `foo[bar]`.
static void subscript(Compiler* compiler, int allowAssignment)
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
    // The VM can only handle a certain number of parameters, so check for
    // this explicitly and give a usable error.
    if (++numArgs == MAX_PARAMETERS + 1)
    {
      // Only show an error at exactly max + 1 and don't break so that we can
      // keep parsing the parameter list and minimize cascaded errors.
      error(compiler, "Cannot pass more than %d arguments to a method.",
            MAX_PARAMETERS);
    }

    expression(compiler);

    // Add a space in the name for each argument. Lets us overload by
    // arity.
    name[length++] = ' ';
  }
  while (match(compiler, TOKEN_COMMA));

  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after arguments.");

  name[length++] = ']';
  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  // TODO(bob): Check for "=" here and handle subscript setters.

  // Compile the method call.
  emit(compiler, CODE_CALL_0 + numArgs);
  emit(compiler, symbol);
}

void call(Compiler* compiler, int allowAssignment)
{
  namedCall(compiler, allowAssignment, CODE_CALL_0);
}

void is(Compiler* compiler, int allowAssignment)
{
  // Compile the right-hand side.
  parsePrecedence(compiler, 0, PREC_CALL);

  emit(compiler, CODE_IS);
}

void and(Compiler* compiler, int allowAssignment)
{
  // Skip the right argument if the left is false.
  emit(compiler, CODE_AND);
  int jump = emit(compiler, 255);

  parsePrecedence(compiler, 0, PREC_LOGIC);

  patchJump(compiler, jump);
}

void or(Compiler* compiler, int allowAssignment)
{
  // Skip the right argument if the left is true.
  emit(compiler, CODE_OR);
  int jump = emit(compiler, 255);

  parsePrecedence(compiler, 0, PREC_LOGIC);

  patchJump(compiler, jump);
}

void infixOp(Compiler* compiler, int allowAssignment)
{
  GrammarRule* rule = &rules[compiler->parser->previous.type];

  // Compile the right-hand side.
  parsePrecedence(compiler, 0, rule->precedence + 1);

  // Call the operator method on the left-hand side.
  int symbol = ensureSymbol(&compiler->parser->vm->methods,
                            rule->name, strlen(rule->name));
  emit(compiler, CODE_CALL_1);
  emit(compiler, symbol);
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
  /* TOKEN_CLASS         */ UNUSED,
  /* TOKEN_ELSE          */ UNUSED,
  /* TOKEN_FALSE         */ PREFIX(boolean),
  /* TOKEN_FN            */ PREFIX(function),
  /* TOKEN_IF            */ UNUSED,
  /* TOKEN_IS            */ INFIX(PREC_IS, is),
  /* TOKEN_NULL          */ PREFIX(null),
  /* TOKEN_RETURN        */ UNUSED,
  /* TOKEN_STATIC        */ UNUSED,
  /* TOKEN_SUPER         */ PREFIX(super_),
  /* TOKEN_THIS          */ PREFIX(this_),
  /* TOKEN_TRUE          */ PREFIX(boolean),
  /* TOKEN_VAR           */ UNUSED,
  /* TOKEN_WHILE         */ UNUSED,
  /* TOKEN_FIELD         */ PREFIX(field),
  /* TOKEN_NAME          */ { name, NULL, parameterList, PREC_NONE, NULL },
  /* TOKEN_NUMBER        */ PREFIX(number),
  /* TOKEN_STRING        */ PREFIX(string),
  /* TOKEN_LINE          */ UNUSED,
  /* TOKEN_ERROR         */ UNUSED,
  /* TOKEN_EOF           */ UNUSED
};

// The main entrypoint for the top-down operator precedence parser.
void parsePrecedence(Compiler* compiler, int allowAssignment,
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
  parsePrecedence(compiler, 1, PREC_LOWEST);
}

// Compiles a method definition inside a class body.
void method(Compiler* compiler, Code instruction, int isConstructor,
            SignatureFn signature)
{
  Compiler methodCompiler;
  int constant = initCompiler(&methodCompiler, compiler->parser, compiler, 1);

  // Build the method name.
  char name[MAX_METHOD_SIGNATURE];
  int length = copyName(compiler, name);

  // Compile the method signature.
  signature(&methodCompiler, name, &length);

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  consume(compiler, TOKEN_LEFT_BRACE, "Expect '{' to begin method body.");

  // If this is a constructor, the first thing is does is create the new
  // instance.
  if (isConstructor) emit(&methodCompiler, CODE_NEW);

  finishBlock(&methodCompiler);
  // TODO(bob): Single-expression methods that implicitly return the result.

  // If it's a constructor, return "this".
  if (isConstructor)
  {
    // The receiver is always stored in the first local slot.
    emit(&methodCompiler, CODE_LOAD_LOCAL);
    emit(&methodCompiler, 0);

    emit(&methodCompiler, CODE_RETURN);
  }
  else
  {
    // End the method's code.
    emit(&methodCompiler, CODE_NULL);
    emit(&methodCompiler, CODE_RETURN);
  }

  endCompiler(&methodCompiler, constant);

  // Compile the code to define the method.
  emit(compiler, instruction);
  emit(compiler, symbol);
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

  // TODO(bob): Only allowing expressions here means you can't do:
  //
  // if (foo) return "blah"
  //
  // since return is a statement (or should it be an expression?).

  // Expression statement.
  expression(compiler);
  emit(compiler, CODE_POP);
}

// Defines a default constructor method with an empty body.
static void defaultConstructor(Compiler* compiler)
{
  Compiler ctorCompiler;
  int constant = initCompiler(&ctorCompiler, compiler->parser, compiler, 1);

  // Create the instance of the class.
  emit(&ctorCompiler, CODE_NEW);

  // Then return the receiver which is in the first local slot.
  emit(&ctorCompiler, CODE_LOAD_LOCAL);
  emit(&ctorCompiler, 0);
  emit(&ctorCompiler, CODE_RETURN);

  endCompiler(&ctorCompiler, constant);

  // Define the constructor method.
  emit(compiler, CODE_METHOD_STATIC);
  emit(compiler, ensureSymbol(&compiler->parser->vm->methods, "new", 3));
}

// Compiles a statement. These can only appear at the top-level or within
// curly blocks. Unlike expressions, these do not leave a value on the stack.
void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_CLASS))
  {
    // Create a variable to store the class in.
    int symbol = declareVariable(compiler);

    // Load the superclass (if there is one).
    if (match(compiler, TOKEN_IS))
    {
      parsePrecedence(compiler, 0, PREC_CALL);
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

    // Classes with no explicitly defined constructor get a default one.
    int hasConstructor = 0;

    // Compile the method definitions.
    consume(compiler, TOKEN_LEFT_BRACE, "Expect '}' after class body.");
    while (!match(compiler, TOKEN_RIGHT_BRACE))
    {
      Code instruction = CODE_METHOD_INSTANCE;
      int isConstructor = 0;

      if (match(compiler, TOKEN_STATIC))
      {
        instruction = CODE_METHOD_STATIC;
        // TODO(bob): Need to handle fields inside static methods correctly.
        // Currently, they're compiled as instance fields, which will be wrong
        // wrong wrong given that the receiver is actually the class obj.
      }
      else if (match(compiler, TOKEN_THIS))
      {
        // If the method name is prefixed with "this", it's a constructor.
        isConstructor = 1;
        hasConstructor = 1;

        // Constructors are defined on the class.
        instruction = CODE_METHOD_STATIC;
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

    if (!hasConstructor) defaultConstructor(compiler);

    // Update the class with the number of fields.
    compiler->fn->bytecode[numFieldsInstruction] = fields.count;

    compiler->fields = previousFields;

    // Store it in its name.
    defineVariable(compiler, symbol);
    return;
  }

  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");

    // Jump to the else branch if the condition is false.
    emit(compiler, CODE_JUMP_IF);
    int ifJump = emit(compiler, 255);

    // Compile the then branch.
    block(compiler);

    // Jump over the else branch when the if branch is taken.
    emit(compiler, CODE_JUMP);
    int elseJump = emit(compiler, 255);

    patchJump(compiler, ifJump);

    // Compile the else branch if there is one.
    if (match(compiler, TOKEN_ELSE))
    {
      block(compiler);
    }

    // Patch the jump over the else.
    patchJump(compiler, elseJump);
    return;
  }

  if (match(compiler, TOKEN_RETURN))
  {
    // Compile the return value.
    // TODO(bob): Implicitly return null if there is a newline or } after the
    // "return".
    expression(compiler);

    emit(compiler, CODE_RETURN);
    return;
  }

  if (match(compiler, TOKEN_VAR))
  {
    // TODO(bob): Variable should not be in scope until after initializer.
    int symbol = declareVariable(compiler);

    // TODO(bob): Allow uninitialized vars?
    consume(compiler, TOKEN_EQ, "Expect '=' after variable name.");

    // Compile the initializer.
    expression(compiler);

    defineVariable(compiler, symbol);
    return;
  }

  if (match(compiler, TOKEN_WHILE))
  {
    // Remember what instruction to loop back to.
    int loopStart = compiler->numCodes - 1;

    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

    emit(compiler, CODE_JUMP_IF);
    int exitJump = emit(compiler, 255);

    // Compile the body.
    block(compiler);

    // Loop back to the top.
    emit(compiler, CODE_LOOP);
    int loopOffset = compiler->numCodes - loopStart;
    emit(compiler, loopOffset);

    patchJump(compiler, exitJump);
    return;
  }

  block(compiler);
}

// Parses [source] to a "function" (a chunk of top-level code) for execution by
// [vm].
ObjFn* wrenCompile(WrenVM* vm, const char* source)
{
  Parser parser;
  parser.vm = vm;
  parser.source = source;
  parser.hasError = 0;

  // Ignore leading newlines.
  parser.skipNewlines = 1;

  parser.tokenStart = source;
  parser.currentChar = source;
  parser.currentLine = 1;

  // Zero-init the current token. This will get copied to previous when
  // advance() is called below.
  parser.current.type = TOKEN_ERROR;
  parser.current.start = source;
  parser.current.length = 0;
  parser.current.line = 0;

  // Read the first token.
  nextToken(&parser);

  Compiler compiler;
  initCompiler(&compiler, &parser, NULL, 0);

  PinnedObj pinned;
  pinObj(vm, (Obj*)compiler.fn, &pinned);

  for (;;)
  {
    statement(&compiler);

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

  endCompiler(&compiler, -1);

  unpinObj(vm);

  return parser.hasError ? NULL : compiler.fn;
}

void wrenBindMethod(ObjClass* classObj, ObjFn* fn)
{
  // TODO(bob): What about functions nested inside [fn]?
  int ip = 0;
  for (;;)
  {
    Code instruction = fn->bytecode[ip++];
    switch (instruction)
    {
        // Instructions with no arguments:
      case CODE_NULL:
      case CODE_FALSE:
      case CODE_TRUE:
      case CODE_DUP:
      case CODE_POP:
      case CODE_IS:
      case CODE_CLOSE_UPVALUE:
      case CODE_RETURN:
      case CODE_NEW:
        break;

        // Instructions with one argument:
      case CODE_CONSTANT:
      case CODE_LOAD_LOCAL:
      case CODE_STORE_LOCAL:
      case CODE_LOAD_UPVALUE:
      case CODE_STORE_UPVALUE:
      case CODE_LOAD_GLOBAL:
      case CODE_STORE_GLOBAL:
      case CODE_CALL_0:
      case CODE_CALL_1:
      case CODE_CALL_2:
      case CODE_CALL_3:
      case CODE_CALL_4:
      case CODE_CALL_5:
      case CODE_CALL_6:
      case CODE_CALL_7:
      case CODE_CALL_8:
      case CODE_CALL_9:
      case CODE_CALL_10:
      case CODE_CALL_11:
      case CODE_CALL_12:
      case CODE_CALL_13:
      case CODE_CALL_14:
      case CODE_CALL_15:
      case CODE_CALL_16:
      case CODE_SUPER_0:
      case CODE_SUPER_1:
      case CODE_SUPER_2:
      case CODE_SUPER_3:
      case CODE_SUPER_4:
      case CODE_SUPER_5:
      case CODE_SUPER_6:
      case CODE_SUPER_7:
      case CODE_SUPER_8:
      case CODE_SUPER_9:
      case CODE_SUPER_10:
      case CODE_SUPER_11:
      case CODE_SUPER_12:
      case CODE_SUPER_13:
      case CODE_SUPER_14:
      case CODE_SUPER_15:
      case CODE_SUPER_16:
      case CODE_JUMP:
      case CODE_LOOP:
      case CODE_JUMP_IF:
      case CODE_AND:
      case CODE_OR:
      case CODE_LIST:
      case CODE_CLASS:
      case CODE_SUBCLASS:
        ip++;
        break;

        // Instructions with two arguments:
      case CODE_METHOD_INSTANCE:
      case CODE_METHOD_STATIC:
        ip += 2;
        break;

      case CODE_CLOSURE:
      {
        int constant = fn->bytecode[ip++];
        ObjFn* loadedFn = AS_FN(fn->constants[constant]);
        ip += loadedFn->numUpvalues;
        break;
      }

      case CODE_LOAD_FIELD:
      case CODE_STORE_FIELD:
        // Shift this class's fields down past the inherited ones.
        fn->bytecode[ip++] += classObj->superclass->numFields;
        break;

      case CODE_END:
        return;

      default:
        ASSERT(0, "Unknown instruction.");
        break;
    }
  }
}
