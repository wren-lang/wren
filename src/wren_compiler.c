#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "wren_common.h"
#include "wren_compiler.h"

// This is written in bottom-up order, so the tokenization comes first, then
// parsing/code generation. This minimizes the number of explicit forward
// declarations needed.

// The maximum length of a method signature. This includes the name, and the
// extra spaces added to handle arity.
#define MAX_METHOD_SIGNATURE 256

// TODO(bob): Get rid of this and use a growable buffer.
#define MAX_STRING (1024)

// The maximum number of arguments that can be passed to a method. Note that
// this limtation is hardcoded in other places in the VM, in particular, the
// `CODE_CALL_XX` instructions assume a certain maximum number.
#define MAX_PARAMETERS (16)

// The maximum number of local (i.e. non-global) variables that can be declared
// in a single function, method, or chunk of top level code. This is the
// maximum number of variables in scope at one time, and spans block scopes.
//
// Note that this limitation is also explicit in the bytecode. Since
// [CODE_LOAD_LOCAL] and [CODE_STORE_LOCAL] use a single argument byte to
// identify the local, only 256 can be in scope at one time.
#define MAX_LOCALS (256)

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
  TOKEN_STATIC,
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
} Local;

typedef struct sCompiler
{
  Parser* parser;

  // The compiler for the block enclosing this one, or NULL if it's the
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
} Compiler;

// Adds [constant] to the constant pool and returns its index.
static int addConstant(Compiler* compiler, Value constant)
{
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

    // The initial scope for function or method is a local scope.
    // TODO(bob): Need to explicitly pop this scope at end of fn/method so
    // that we can correctly close over locals declared at top level of member.
    // also, when done compiling fn/method, need to count total number of
    // upvalues and store in fnobj. note: have to make sure we include upvalues
    // added because a fn within this one closed over something outside of this
    // one and we had to add upvalue here to flatten the closure.
    compiler->scopeDepth = 0;
  }

  // Propagate the enclosing class downwards.
  compiler->fields = parent != NULL ? parent->fields :  NULL;

  compiler->fn = wrenNewFunction(parser->vm);
  compiler->fn->numConstants = 0;

  if (parent == NULL) return -1;

  // Add the block to the constant table. Do this eagerly so it's reachable by
  // the GC.
  return addConstant(parent, OBJ_VAL(compiler->fn));
}

// Emits one bytecode instruction or argument.
static int emit(Compiler* compiler, Code code)
{
  compiler->fn->bytecode[compiler->numCodes++] = code;
  return compiler->numCodes - 1;
}

// Finishes [compiler], which is compiling a function, method, or chunk of top
// level code. If there is a parent compiler, then this emits code in the
// parent compiler to load the resulting function.
static void endCompiler(Compiler* compiler, int constant)
{
  // End the function's code.
  emit(compiler, CODE_END);

  // TODO(bob): will need to compile different code to capture upvalues if fn
  // has them.
  if (compiler->parent != NULL)
  {
    // In the function that contains this one, load the resulting function object.
    emit(compiler->parent, CODE_CONSTANT);
    emit(compiler->parent, constant);
  }
}

// Outputs a compile or syntax error.
static void error(Compiler* compiler, const char* format, ...)
{
  compiler->parser->hasError = 1;

  Token* token = &compiler->parser->previous;
  fprintf(stderr, "[Line %d] Error on '%.*s': ",
          token->line, token->length, token->start);

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
  if (isKeyword(parser, "static")) type = TOKEN_STATIC;
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

        if (peekChar(parser) == '*')
        {
          skipBlockComment(parser);
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
  return compiler->numLocals++;
}

// Stores a variable with the previously defined symbol in the current scope.
static void defineVariable(Compiler* compiler, int symbol)
{
  // Handle top-level global scope.
  if (compiler->scopeDepth == -1)
  {
    // It's a global variable, so store the value in the global slot.
    emit(compiler, CODE_STORE_GLOBAL);
    emit(compiler, symbol);
  }
  else
  {
    // It's a local variable. The value is already in the right slot to store
    // the local, but later code will pop and discard that. To cancel that out
    // duplicate it now, so that the temporary value will be discarded and
    // leave the local still on the stack.
    // TODO(bob): Since variables are declared in statement position, this
    // generates a lot of code like:
    //
    //   var a = "value"
    //   io.write(a)
    //
    //   CODE_CONSTANT "value" // put constant into local slot
    //   CODE_DUP              // dup it so the top is a temporary
    //   CODE_POP              // discard previous result in sequence
    //   <code for io.write...>
    //
    // Would be good to either peephole optimize this or be smarter about
    // generating code for defining local variables to not emit the DUP
    // sometimes.
    emit(compiler, CODE_DUP);
  }
}

// Starts a new local block scope.
static void pushScope(Compiler* compiler)
{
  compiler->scopeDepth++;
}

// Closes the last pushed block scope.
static void popScope(Compiler* compiler)
{
  ASSERT(compiler->scopeDepth > -1, "Cannot pop top-level scope.");

  // Pop locals off the stack.
  // TODO(bob): Could make a single instruction that pops multiple values if
  // this is a bottleneck.
  while (compiler->numLocals > 0 &&
         compiler->locals[compiler->numLocals - 1].depth ==
            compiler->scopeDepth)
  {
    compiler->numLocals--;
    emit(compiler, CODE_POP);
  }

  // TODO(bob): Need to emit code to capture upvalue for any local going out of
  // scope now that is closed over.
  
  compiler->scopeDepth--;
}

// Look up the previously consumed token, which is presumed to be a TOKEN_NAME
// in the current scope to see what name it is bound to. Returns the index of
// the name either in global or local scope. Returns -1 if not found. Sets
// [isGlobal] to non-zero if the name is in global scope, or 0 if in local.
static int resolveName(Compiler* compiler, int* isGlobal)
{
  Token* token = &compiler->parser->previous;

  // Look it up in the local scopes. Look in reverse order so that the most
  // nested variable is found first and shadows outer ones.
  *isGlobal = 0;
  for (int i = compiler->numLocals - 1; i >= 0; i--)
  {
    if (compiler->locals[i].length == token->length &&
        strncmp(token->start, compiler->locals[i].name, token->length) == 0)
    {
      return i;
    }
  }

  // TODO(bob): Closures!
  // look in current upvalues to see if we've already closed over it
  //   if so, just use that
  // walk up parent chain looking in their local scopes for variable
  // if we find it, need to close over it here
  // add upvalue to fn being compiled
  //   return index of upvalue
  // instead of isGlobal, should be some local/upvalue/global enum

  // If we got here, it wasn't in a local scope, so try the global scope.
  *isGlobal = 1;
  return findSymbol(&compiler->parser->vm->globalSymbols,
                    token->start, token->length);
}

// Copies the identifier from the previously consumed `TOKEN_NAME` into [name],
// which should point to a buffer large enough to contain it. Returns the
// length of the name.
static int copyName(Compiler* compiler, char* name)
{
  Token* token = &compiler->parser->previous;
  strncpy(name, token->start, token->length);
  return token->length;
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
static void expression(Compiler* compiler, int allowAssignment);
static void assignment(Compiler* compiler);
static void statement(Compiler* compiler);
static void definition(Compiler* compiler);
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
    definition(compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(compiler, TOKEN_LINE))
    {
      consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after block body.");
      break;
    }

    if (match(compiler, TOKEN_RIGHT_BRACE)) break;

    // Discard the result of the previous expression.
    emit(compiler, CODE_POP);
  }
}

// Parses an optional parenthesis-delimited parameter list.
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
      // TODO(bob): How can name be null?
      if (name != NULL) name[(*length)++] = ' ';
      // TODO(bob): Check for length overflow.
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
  }
}

static void grouping(Compiler* compiler, int allowAssignment)
{
  assignment(compiler);
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
      assignment(compiler);
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
  }
  else
  {
    // Single expression body.
    // TODO(bob): Allow assignment here?
    expression(&fnCompiler, 0);
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
    statement(compiler);

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
  int isGlobal;
  int index = resolveName(compiler, &isGlobal);

  if (index == -1)
  {
    error(compiler, "Undefined variable.");
  }

  // If there's an "=" after a bare name, it's a variable assignment.
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    statement(compiler);

    // TODO(bob): Handle assigning to upvalue.

    if (isGlobal)
    {
      emit(compiler, CODE_STORE_GLOBAL);
      emit(compiler, index);
    }
    else
    {
      emit(compiler, CODE_STORE_LOCAL);
      emit(compiler, index);
    }

    return;
  }

  // TODO(bob): Handle reading upvalue.

  // Otherwise, it's just a variable access.
  if (isGlobal)
  {
    emit(compiler, CODE_LOAD_GLOBAL);
    emit(compiler, index);
  }
  else
  {
    emit(compiler, CODE_LOAD_LOCAL);
    emit(compiler, index);
  }
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

    statement(compiler);

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
  // Build the method name.
  consume(compiler, TOKEN_NAME, "Expect method name after '.'.");
  char name[MAX_METHOD_SIGNATURE];
  int length = copyName(compiler, name);
  // TODO(bob): Check for length overflow.

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
      
      statement(compiler);

      // Add a space in the name for each argument. Lets us overload by
      // arity.
      name[length++] = ' ';
    }
    while (match(compiler, TOKEN_COMMA));
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  }

  int symbol = ensureSymbol(&compiler->parser->vm->methods, name, length);

  // Compile the method call.
  emit(compiler, CODE_CALL_0 + numArgs);
  emit(compiler, symbol);
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
  /* TOKEN_MINUS         */ { unaryOp, infixOp, mixedSignature, PREC_TERM, "- " },
  /* TOKEN_PIPE          */ UNUSED,
  /* TOKEN_PIPEPIPE      */ INFIX(PREC_LOGIC, or),
  /* TOKEN_AMP           */ UNUSED,
  /* TOKEN_AMPAMP        */ INFIX(PREC_LOGIC, and),
  /* TOKEN_BANG          */ PREFIX_OPERATOR("!"),
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
  /* TOKEN_STATIC        */ UNUSED,
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

// Parses an expression (or, really, the subset of expressions that can appear
// outside of the top level of a block). Does not include "statement-like"
// things like variable declarations.
void expression(Compiler* compiler, int allowAssignment)
{
  parsePrecedence(compiler, allowAssignment, PREC_LOWEST);
}

// Compiles an assignment expression.
void assignment(Compiler* compiler)
{
  // Assignment statement.
  expression(compiler, 1);
}

// Parses a "statement": any expression including expressions like variable
// declarations which can only appear at the top level of a block.
void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    assignment(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");

    // Jump to the else branch if the condition is false.
    emit(compiler, CODE_JUMP_IF);
    int ifJump = emit(compiler, 255);

    // Compile the then branch.
    pushScope(compiler);
    definition(compiler);
    popScope(compiler);

    // Jump over the else branch when the if branch is taken.
    emit(compiler, CODE_JUMP);
    int elseJump = emit(compiler, 255);

    patchJump(compiler, ifJump);

    // Compile the else branch if there is one.
    if (match(compiler, TOKEN_ELSE))
    {
      pushScope(compiler);
      definition(compiler);
      popScope(compiler);
    }
    else
    {
      // Just default to null.
      emit(compiler, CODE_NULL);
    }

    // Patch the jump over the else.
    patchJump(compiler, elseJump);
    return;
  }

  if (match(compiler, TOKEN_WHILE))
  {
    // Remember what instruction to loop back to.
    int loopStart = compiler->numCodes - 1;

    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    assignment(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

    emit(compiler, CODE_JUMP_IF);
    int exitJump = emit(compiler, 255);

    // Compile the body.
    pushScope(compiler);
    definition(compiler);
    popScope(compiler);

    // Loop back to the top.
    emit(compiler, CODE_LOOP);
    int loopOffset = compiler->numCodes - loopStart;
    emit(compiler, loopOffset);

    patchJump(compiler, exitJump);

    // A while loop always evaluates to null.
    emit(compiler, CODE_NULL);
    return;
  }

  // Curly block.
  if (match(compiler, TOKEN_LEFT_BRACE))
  {
    pushScope(compiler);
    finishBlock(compiler);
    popScope(compiler);
    return;
  }

  assignment(compiler);
}

// Compiles a method definition inside a class body.
void method(Compiler* compiler, Code instruction, SignatureFn signature)
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
  finishBlock(&methodCompiler);

  // If it's a constructor, return "this", not the result of the body.
  if (instruction == CODE_METHOD_CTOR)
  {
    emit(&methodCompiler, CODE_POP);
    // The receiver is always stored in the first local slot.
    // TODO(bob): Will need to do something different to handle functions
    // enclosed in methods.
    emit(&methodCompiler, CODE_LOAD_LOCAL);
    emit(&methodCompiler, 0);
  }

  endCompiler(&methodCompiler, constant);

  // Compile the code to define the method.
  emit(compiler, instruction);
  emit(compiler, symbol);
}

// Compiles a name-binding statement.
void definition(Compiler* compiler)
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

    // Store it in its name.
    defineVariable(compiler, symbol);

    // Compile the method definitions.
    consume(compiler, TOKEN_LEFT_BRACE, "Expect '}' after class body.");

    // Set up a symbol table for the class's fields.
    SymbolTable* previousFields = compiler->fields;
    SymbolTable fields;
    initSymbolTable(&fields);
    compiler->fields = &fields;

    // TODO(bob): Need to handle inherited fields. Ideally, a subclass's fields
    // would be statically compiled to slot indexes right after the superclass
    // ones, but we don't know the superclass statically. Instead, will
    // probably have to determine the field offset at class creation time in
    // the VM and then adjust by that every time a field is accessed/modified.

    while (!match(compiler, TOKEN_RIGHT_BRACE))
    {
      Code instruction = CODE_METHOD_INSTANCE;
      if (match(compiler, TOKEN_STATIC))
      {
        instruction = CODE_METHOD_STATIC;
        // TODO(bob): Need to handle fields inside static methods correctly.
        // Currently, they're compiled as instance fields, which will be wrong
        // wrong wrong given that the receiver is actually the class obj.
      }
      else if (match(compiler, TOKEN_THIS))
      {
        // If the method name is prefixed with "this", it's a named constructor.
        // TODO(bob): Allow defining unnamed constructor.
        instruction = CODE_METHOD_CTOR;
      }

      SignatureFn signature = rules[compiler->parser->current.type].method;
      nextToken(compiler->parser);

      if (signature == NULL)
      {
        error(compiler, "Expect method definition.");
        break;
      }

      method(compiler, instruction, signature);
      consume(compiler, TOKEN_LINE,
              "Expect newline after definition in class.");
    }

    // Update the class with the number of fields.
    compiler->fn->bytecode[numFieldsInstruction] = fields.count;

    compiler->fields = previousFields;
    return;
  }

  if (match(compiler, TOKEN_VAR))
  {
    // TODO(bob): Variable should not be in scope until after initializer.
    int symbol = declareVariable(compiler);

    // TODO(bob): Allow uninitialized vars?
    consume(compiler, TOKEN_EQ, "Expect '=' after variable name.");

    // Compile the initializer.
    statement(compiler);

    defineVariable(compiler, symbol);
    return;
  }

  statement(compiler);
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
    definition(&compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!match(&compiler, TOKEN_LINE))
    {
      consume(&compiler, TOKEN_EOF, "Expect end of file.");
      break;
    }

    if (match(&compiler, TOKEN_EOF)) break;

    // Discard the result of the previous expression.
    emit(&compiler, CODE_POP);
  }

  endCompiler(&compiler, -1);

  unpinObj(vm);

  return parser.hasError ? NULL : compiler.fn;
}
