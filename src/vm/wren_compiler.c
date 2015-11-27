#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_vm.h"

#if WREN_DEBUG_DUMP_COMPILED_CODE
  #include "wren_debug.h"
#endif

// This is written in bottom-up order, so the tokenization comes first, then
// parsing/code generation. This minimizes the number of explicit forward
// declarations needed.

// The maximum number of local (i.e. not module level) variables that can be
// declared in a single function, method, or chunk of top level code. This is
// the maximum number of variables in scope at one time, and spans block scopes.
//
// Note that this limitation is also explicit in the bytecode. Since
// `CODE_LOAD_LOCAL` and `CODE_STORE_LOCAL` use a single argument byte to
// identify the local, only 256 can be in scope at one time.
#define MAX_LOCALS 256

// The maximum number of upvalues (i.e. variables from enclosing functions)
// that a function can close over.
#define MAX_UPVALUES 256

// The maximum number of distinct constants that a function can contain. This
// value is explicit in the bytecode since `CODE_CONSTANT` only takes a single
// two-byte argument.
#define MAX_CONSTANTS (1 << 16)

// The maximum depth that interpolation can nest. For example, this string has
// three levels:
//
//      "outside %(one + "%(two + "%(three)")")"
#define MAX_INTERPOLATION_NESTING 8

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
  TOKEN_LTLT,
  TOKEN_GTGT,
  TOKEN_PIPE,
  TOKEN_PIPEPIPE,
  TOKEN_CARET,
  TOKEN_AMP,
  TOKEN_AMPAMP,
  TOKEN_BANG,
  TOKEN_TILDE,
  TOKEN_QUESTION,
  TOKEN_EQ,
  TOKEN_LT,
  TOKEN_GT,
  TOKEN_LTEQ,
  TOKEN_GTEQ,
  TOKEN_EQEQ,
  TOKEN_BANGEQ,

  TOKEN_BREAK,
  TOKEN_CLASS,
  TOKEN_CONSTRUCT,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FOREIGN,
  TOKEN_IF,
  TOKEN_IMPORT,
  TOKEN_IN,
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
  TOKEN_STATIC_FIELD,
  TOKEN_NAME,
  TOKEN_NUMBER,
  
  // A string literal without any interpolation, or the last section of a
  // string following the last interpolated expression.
  TOKEN_STRING,
  
  // A portion of a string literal preceding an interpolated expression. This
  // string:
  //
  //     "a %(b) c %(d) e"
  //
  // is tokenized to:
  //
  //     TOKEN_INTERPOLATION "a "
  //     TOKEN_NAME          b
  //     TOKEN_INTERPOLATION " c "
  //     TOKEN_NAME          d
  //     TOKEN_STRING        " e"
  TOKEN_INTERPOLATION,

  TOKEN_LINE,

  TOKEN_ERROR,
  TOKEN_EOF
} TokenType;

typedef struct
{
  TokenType type;

  // The beginning of the token, pointing directly into the source.
  const char* start;

  // The length of the token in characters.
  int length;

  // The 1-based line where the token appears.
  int line;
  
  // The parsed value if the token is a literal.
  Value value;
} Token;

typedef struct
{
  WrenVM* vm;

  // The module being parsed.
  ObjModule* module;

  // The source code being parsed.
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
  
  // Tracks the lexing state when tokenizing interpolated strings.
  //
  // Interpolated strings make the lexer not strictly regular: we don't know
  // whether a ")" should be treated as a RIGHT_PAREN token or as ending an
  // interpolated expression unless we know whether we are inside a string
  // interpolation and how many unmatched "(" there are. This is particularly
  // complex because interpolation can nest:
  //
  //     " %( " %( inner ) " ) "
  //
  // This tracks that state. The parser maintains a stack of ints, one for each
  // level of current interpolation nesting. Each value is the number of
  // unmatched "(" that are waiting to be closed.
  int parens[MAX_INTERPOLATION_NESTING];
  int numParens;
  
  // If subsequent newline tokens should be discarded.
  bool skipNewlines;

  // Whether compile errors should be printed to stderr or discarded.
  bool printErrors;

  // If a syntax or compile error has occurred.
  bool hasError;
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

// Bookkeeping information for the current loop being compiled.
typedef struct sLoop
{
  // Index of the instruction that the loop should jump back to.
  int start;

  // Index of the argument for the CODE_JUMP_IF instruction used to exit the
  // loop. Stored so we can patch it once we know where the loop ends.
  int exitJump;

  // Index of the first instruction of the body of the loop.
  int body;

  // Depth of the scope(s) that need to be exited if a break is hit inside the
  // loop.
  int scopeDepth;

  // The loop enclosing this one, or NULL if this is the outermost loop.
  struct sLoop* enclosing;
} Loop;

// The different signature syntaxes for different kinds of methods.
typedef enum
{
  // A name followed by a (possibly empty) parenthesized parameter list. Also
  // used for binary operators.
  SIG_METHOD,
  
  // Just a name. Also used for unary operators.
  SIG_GETTER,
  
  // A name followed by "=".
  SIG_SETTER,
  
  // A square bracketed parameter list.
  SIG_SUBSCRIPT,
  
  // A square bracketed parameter list followed by "=".
  SIG_SUBSCRIPT_SETTER,
  
  // A constructor initializer function. This has a distinct signature to
  // prevent it from being invoked directly outside of the constructor on the
  // metaclass.
  SIG_INITIALIZER
} SignatureType;

typedef struct
{
  const char* name;
  int length;
  SignatureType type;
  int arity;
} Signature;

// Bookkeeping information for compiling a class definition.
typedef struct
{
  // Symbol table for the fields of the class.
  SymbolTable* fields;

  // True if the class being compiled is a foreign class.
  bool isForeign;
  
  // True if the current method being compiled is static.
  bool inStatic;

  // The signature of the method being compiled.
  Signature* signature;
} ClassCompiler;

struct sCompiler
{
  Parser* parser;

  // The compiler for the function enclosing this one, or NULL if it's the
  // top level.
  struct sCompiler* parent;

  // The constants that have been defined in this function so far.
  ValueBuffer constants;

  // The currently in scope local variables.
  Local locals[MAX_LOCALS];

  // The number of local variables currently in scope.
  int numLocals;

  // The upvalues that this function has captured from outer scopes. The count
  // of them is stored in [numUpvalues].
  CompilerUpvalue upvalues[MAX_UPVALUES];

  int numUpvalues;

  // The number of parameters this method or function expects.
  int numParams;

  // The current level of block scope nesting, where zero is no nesting. A -1
  // here means top-level code is being compiled and there is no block scope
  // in effect at all. Any variables declared will be module-level.
  int scopeDepth;

  // The current innermost loop being compiled, or NULL if not in a loop.
  Loop* loop;

  // If this is a compiler for a method, keeps track of the class enclosing it.
  ClassCompiler* enclosingClass;

  // The growable buffer of code that's been compiled so far.
  ByteBuffer bytecode;

  // The growable buffer of source line mappings.
  IntBuffer debugSourceLines;
};

// Outputs a compile or syntax error. This also marks the compilation as having
// an error, which ensures that the resulting code will be discarded and never
// run. This means that after calling lexError(), it's fine to generate whatever
// invalid bytecode you want since it won't be used.
static void lexError(Parser* parser, const char* format, ...)
{
  parser->hasError = true;
  if (!parser->printErrors) return;

  fprintf(stderr, "[%s line %d] Error: ",
          parser->module->name->value, parser->currentLine);

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
  if (!compiler->parser->printErrors) return;

  Token* token = &compiler->parser->previous;

  // If the parse error was caused by an error token, the lexer has already
  // reported it.
  if (token->type == TOKEN_ERROR) return;

  fprintf(stderr, "[%s line %d] Error at ",
          compiler->parser->module->name->value, token->line);

  if (token->type == TOKEN_LINE)
  {
    fprintf(stderr, "newline: ");
  }
  else if (token->type == TOKEN_EOF)
  {
    fprintf(stderr, "end of file: ");
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
  if (compiler->parser->hasError) return -1;
  
  if (compiler->constants.count < MAX_CONSTANTS)
  {
    if (IS_OBJ(constant)) wrenPushRoot(compiler->parser->vm, AS_OBJ(constant));
    wrenValueBufferWrite(compiler->parser->vm, &compiler->constants, constant);
    if (IS_OBJ(constant)) wrenPopRoot(compiler->parser->vm);
  }
  else
  {
    error(compiler, "A function may only contain %d unique constants.",
          MAX_CONSTANTS);
  }

  return compiler->constants.count - 1;
}

// Initializes [compiler].
static void initCompiler(Compiler* compiler, Parser* parser, Compiler* parent,
                         bool isFunction)
{
  compiler->parser = parser;
  compiler->parent = parent;

  // Initialize this to NULL before allocating in case a GC gets triggered in
  // the middle of initializing the compiler.
  wrenValueBufferInit(&compiler->constants);

  compiler->numUpvalues = 0;
  compiler->numParams = 0;
  compiler->loop = NULL;
  compiler->enclosingClass = NULL;

  wrenSetCompiler(parser->vm, compiler);

  if (parent == NULL)
  {
    compiler->numLocals = 0;

    // Compiling top-level code, so the initial scope is module-level.
    compiler->scopeDepth = -1;
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
    if (isFunction)
    {
      compiler->locals[0].name = NULL;
      compiler->locals[0].length = 0;
    }
    else
    {
      compiler->locals[0].name = "this";
      compiler->locals[0].length = 4;
    }
    compiler->locals[0].depth = -1;
    compiler->locals[0].isUpvalue = false;

    // The initial scope for function or method is a local scope.
    compiler->scopeDepth = 0;
  }

  wrenByteBufferInit(&compiler->bytecode);
  wrenIntBufferInit(&compiler->debugSourceLines);
}

// Lexing ----------------------------------------------------------------------

typedef struct
{
  const char* identifier;
  size_t      length;
  TokenType   tokenType;
} Keyword;

// The table of reserved words and their associated token types.
static Keyword keywords[] =
{
  {"break",     5, TOKEN_BREAK},
  {"class",     5, TOKEN_CLASS},
  {"construct", 9, TOKEN_CONSTRUCT},
  {"else",      4, TOKEN_ELSE},
  {"false",     5, TOKEN_FALSE},
  {"for",       3, TOKEN_FOR},
  {"foreign",   7, TOKEN_FOREIGN},
  {"if",        2, TOKEN_IF},
  {"import",    6, TOKEN_IMPORT},
  {"in",        2, TOKEN_IN},
  {"is",        2, TOKEN_IS},
  {"null",      4, TOKEN_NULL},
  {"return",    6, TOKEN_RETURN},
  {"static",    6, TOKEN_STATIC},
  {"super",     5, TOKEN_SUPER},
  {"this",      4, TOKEN_THIS},
  {"true",      4, TOKEN_TRUE},
  {"var",       3, TOKEN_VAR},
  {"while",     5, TOKEN_WHILE},
  {NULL,        0, TOKEN_EOF} // Sentinel to mark the end of the array.
};

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

// If the current character is [c], consumes it and returns `true`.
static bool matchChar(Parser* parser, char c)
{
  if (peekChar(parser) != c) return false;
  nextChar(parser);
  return true;
}

// Sets the parser's current token to the given [type] and current character
// range.
static void makeToken(Parser* parser, TokenType type)
{
  parser->current.type = type;
  parser->current.start = parser->tokenStart;
  parser->current.length = (int)(parser->currentChar - parser->tokenStart);
  parser->current.line = parser->currentLine;
  
  // Make line tokens appear on the line containing the "\n".
  if (type == TOKEN_LINE) parser->current.line--;
}

// If the current character is [c], then consumes it and makes a token of type
// [two]. Otherwise makes a token of type [one].
static void twoCharToken(Parser* parser, char c, TokenType two, TokenType one)
{
  makeToken(parser, matchChar(parser, c) ? two : one);
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

// Reads the next character, which should be a hex digit (0-9, a-f, or A-F) and
// returns its numeric value. If the character isn't a hex digit, returns -1.
static int readHexDigit(Parser* parser)
{
  char c = nextChar(parser);
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;

  // Don't consume it if it isn't expected. Keeps us from reading past the end
  // of an unterminated string.
  parser->currentChar--;
  return -1;
}

// Parses the numeric value of the current token.
static void makeNumber(Parser* parser, bool isHex)
{
  errno = 0;

  // We don't check that the entire token is consumed because we've already
  // scanned it ourselves and know it's valid.
  parser->current.value = NUM_VAL(isHex ? strtol(parser->tokenStart, NULL, 16)
                                        : strtod(parser->tokenStart, NULL));
  
  if (errno == ERANGE)
  {
    lexError(parser, "Number literal was too large.");
    parser->current.value = NUM_VAL(0);
  }

  makeToken(parser, TOKEN_NUMBER);
}

// Finishes lexing a hexadecimal number literal.
static void readHexNumber(Parser* parser)
{
  // Skip past the `x` used to denote a hexadecimal literal.
  nextChar(parser);

  // Iterate over all the valid hexadecimal digits found.
  while (readHexDigit(parser) != -1) continue;

  makeNumber(parser, true);
}

// Finishes lexing a number literal.
static void readNumber(Parser* parser)
{
  while (isDigit(peekChar(parser))) nextChar(parser);

  // See if it has a floating point. Make sure there is a digit after the "."
  // so we don't get confused by method calls on number literals.
  if (peekChar(parser) == '.' && isDigit(peekNextChar(parser)))
  {
    nextChar(parser);
    while (isDigit(peekChar(parser))) nextChar(parser);
  }
  
  // See if the number is in scientific notation.
  if (matchChar(parser, 'e') || matchChar(parser, 'E'))
  {
    // Allow a negative exponent.
    matchChar(parser, '-');
    
    if (!isDigit(peekChar(parser)))
    {
      lexError(parser, "Unterminated scientific notation.");
    }
    
    while (isDigit(peekChar(parser))) nextChar(parser);
  }

  makeNumber(parser, false);
}

// Finishes lexing an identifier. Handles reserved words.
static void readName(Parser* parser, TokenType type)
{
  while (isName(peekChar(parser)) || isDigit(peekChar(parser)))
  {
    nextChar(parser);
  }

  // Update the type if it's a keyword.
  size_t length = parser->currentChar - parser->tokenStart;
  for (int i = 0; keywords[i].identifier != NULL; i++)
  {
    if (length == keywords[i].length &&
        memcmp(parser->tokenStart, keywords[i].identifier, length) == 0)
    {
      type = keywords[i].tokenType;
      break;
    }
  }
  
  makeToken(parser, type);
}

// Reads [digits] hex digits in a string literal and returns their number value.
static int readHexEscape(Parser* parser, int digits, const char* description)
{
  int value = 0;
  for (int i = 0; i < digits; i++)
  {
    if (peekChar(parser) == '"' || peekChar(parser) == '\0')
    {
      lexError(parser, "Incomplete %s escape sequence.", description);

      // Don't consume it if it isn't expected. Keeps us from reading past the
      // end of an unterminated string.
      parser->currentChar--;
      break;
    }

    int digit = readHexDigit(parser);
    if (digit == -1)
    {
      lexError(parser, "Invalid %s escape sequence.", description);
      break;
    }

    value = (value * 16) | digit;
  }

  return value;
}

// Reads a hex digit Unicode escape sequence in a string literal.
static void readUnicodeEscape(Parser* parser, ByteBuffer* string, int length)
{
  int value = readHexEscape(parser, length, "Unicode");

  // Grow the buffer enough for the encoded result.
  int numBytes = wrenUtf8EncodeNumBytes(value);
  if (numBytes != 0)
  {
    wrenByteBufferFill(parser->vm, string, 0, numBytes);
    wrenUtf8Encode(value, string->data + string->count - numBytes);
  }
}

// Finishes lexing a string literal.
static void readString(Parser* parser)
{
  ByteBuffer string;
  TokenType type = TOKEN_STRING;
  wrenByteBufferInit(&string);
  
  for (;;)
  {
    char c = nextChar(parser);
    if (c == '"') break;

    if (c == '\0')
    {
      lexError(parser, "Unterminated string.");

      // Don't consume it if it isn't expected. Keeps us from reading past the
      // end of an unterminated string.
      parser->currentChar--;
      break;
    }

    if (c == '%')
    {
      if (parser->numParens < MAX_INTERPOLATION_NESTING)
      {
        // TODO: Allow format string.
        if (nextChar(parser) != '(') lexError(parser, "Expect '(' after '%'.");
        
        parser->parens[parser->numParens] = 1;
        parser->numParens++;
        type = TOKEN_INTERPOLATION;
        break;
      }

      lexError(parser, "Interpolation may only nest %d levels deep.",
               MAX_INTERPOLATION_NESTING);
    }
    
    if (c == '\\')
    {
      switch (nextChar(parser))
      {
        case '"':  wrenByteBufferWrite(parser->vm, &string, '"'); break;
        case '\\': wrenByteBufferWrite(parser->vm, &string, '\\'); break;
        case '%':  wrenByteBufferWrite(parser->vm, &string, '%'); break;
        case '0':  wrenByteBufferWrite(parser->vm, &string, '\0'); break;
        case 'a':  wrenByteBufferWrite(parser->vm, &string, '\a'); break;
        case 'b':  wrenByteBufferWrite(parser->vm, &string, '\b'); break;
        case 'f':  wrenByteBufferWrite(parser->vm, &string, '\f'); break;
        case 'n':  wrenByteBufferWrite(parser->vm, &string, '\n'); break;
        case 'r':  wrenByteBufferWrite(parser->vm, &string, '\r'); break;
        case 't':  wrenByteBufferWrite(parser->vm, &string, '\t'); break;
        case 'u':  readUnicodeEscape(parser, &string, 4); break;
        case 'U':  readUnicodeEscape(parser, &string, 8); break;
        case 'v':  wrenByteBufferWrite(parser->vm, &string, '\v'); break;
        case 'x':
          wrenByteBufferWrite(parser->vm, &string,
                              (uint8_t)readHexEscape(parser, 2, "byte"));
          break;

        default:
          lexError(parser, "Invalid escape character '%c'.",
                   *(parser->currentChar - 1));
          break;
      }
    }
    else
    {
      wrenByteBufferWrite(parser->vm, &string, c);
    }
  }

  parser->current.value = wrenNewString(parser->vm,
                                        (char*)string.data, string.count);
  
  wrenByteBufferClear(parser->vm, &string);
  makeToken(parser, type);
}

// Lex the next token and store it in [parser.current].
static void nextToken(Parser* parser)
{
  parser->previous = parser->current;

  // If we are out of tokens, don't try to tokenize any more. We *do* still
  // copy the TOKEN_EOF to previous so that code that expects it to be consumed
  // will still work.
  if (parser->current.type == TOKEN_EOF) return;
  
  while (peekChar(parser) != '\0')
  {
    parser->tokenStart = parser->currentChar;

    char c = nextChar(parser);
    switch (c)
    {
      case '(':
        // If we are inside an interpolated expression, count the unmatched "(".
        if (parser->numParens > 0) parser->parens[parser->numParens - 1]++;
        makeToken(parser, TOKEN_LEFT_PAREN);
        return;
        
      case ')':
        // If we are inside an interpolated expression, count the ")".
        if (parser->numParens > 0 &&
            --parser->parens[parser->numParens - 1] == 0)
        {
          // This is the final ")", so the interpolation expression has ended.
          // This ")" now begins the next section of the template string.
          parser->numParens--;
          readString(parser);
          return;
        }
        
        makeToken(parser, TOKEN_RIGHT_PAREN);
        return;
        
      case '[': makeToken(parser, TOKEN_LEFT_BRACKET); return;
      case ']': makeToken(parser, TOKEN_RIGHT_BRACKET); return;
      case '{': makeToken(parser, TOKEN_LEFT_BRACE); return;
      case '}': makeToken(parser, TOKEN_RIGHT_BRACE); return;
      case ':': makeToken(parser, TOKEN_COLON); return;
      case ',': makeToken(parser, TOKEN_COMMA); return;
      case '*': makeToken(parser, TOKEN_STAR); return;
      case '%': makeToken(parser, TOKEN_PERCENT); return;
      case '^': makeToken(parser, TOKEN_CARET); return;
      case '+': makeToken(parser, TOKEN_PLUS); return;
      case '-': makeToken(parser, TOKEN_MINUS); return;
      case '~': makeToken(parser, TOKEN_TILDE); return;
      case '?': makeToken(parser, TOKEN_QUESTION); return;
        
      case '|': twoCharToken(parser, '|', TOKEN_PIPEPIPE, TOKEN_PIPE); return;
      case '&': twoCharToken(parser, '&', TOKEN_AMPAMP, TOKEN_AMP); return;
      case '=': twoCharToken(parser, '=', TOKEN_EQEQ, TOKEN_EQ); return;
      case '!': twoCharToken(parser, '=', TOKEN_BANGEQ, TOKEN_BANG); return;
        
      case '.':
        if (matchChar(parser, '.'))
        {
          twoCharToken(parser, '.', TOKEN_DOTDOTDOT, TOKEN_DOTDOT);
          return;
        }
        
        makeToken(parser, TOKEN_DOT);
        return;
        
      case '/':
        if (matchChar(parser, '/'))
        {
          skipLineComment(parser);
          break;
        }

        if (matchChar(parser, '*'))
        {
          skipBlockComment(parser);
          break;
        }

        makeToken(parser, TOKEN_SLASH);
        return;

      case '<':
        if (matchChar(parser, '<'))
        {
          makeToken(parser, TOKEN_LTLT);
        }
        else
        {
          twoCharToken(parser, '=', TOKEN_LTEQ, TOKEN_LT);
        }
        return;

      case '>':
        if (matchChar(parser, '>'))
        {
          makeToken(parser, TOKEN_GTGT);
        }
        else
        {
          twoCharToken(parser, '=', TOKEN_GTEQ, TOKEN_GT);
        }
        return;

      case '\n':
        makeToken(parser, TOKEN_LINE);
        return;

      case ' ':
      case '\r':
      case '\t':
        // Skip forward until we run out of whitespace.
        while (peekChar(parser) == ' ' ||
               peekChar(parser) == '\r' ||
               peekChar(parser) == '\t')
        {
          nextChar(parser);
        }
        break;

      case '"': readString(parser); return;
      case '_':
        readName(parser,
                 peekChar(parser) == '_' ? TOKEN_STATIC_FIELD : TOKEN_FIELD);
        return;

      case '#':
        // Ignore shebang on the first line.
        if (peekChar(parser) == '!' && parser->currentLine == 1)
        {
          skipLineComment(parser);
          break;
        }

        lexError(parser, "Invalid character '%c'.", c);
        return;

      case '0':
        if (peekChar(parser) == 'x')
        {
          readHexNumber(parser);
          return;
        }

        readNumber(parser);
        return;

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
static void consume(Compiler* compiler, TokenType expected,
                    const char* errorMessage)
{
  nextToken(compiler->parser);
  if (compiler->parser->previous.type != expected)
  {
    error(compiler, errorMessage);

    // If the next token is the one we want, assume the current one is just a
    // spurious error and discard it to minimize the number of cascaded errors.
    if (compiler->parser->current.type == expected) nextToken(compiler->parser);
  }
}

// Matches one or more newlines. Returns true if at least one was found.
static bool matchLine(Compiler* compiler)
{
  if (!match(compiler, TOKEN_LINE)) return false;

  while (match(compiler, TOKEN_LINE));
  return true;
}

// Discards any newlines starting at the current token.
static void ignoreNewlines(Compiler* compiler)
{
  matchLine(compiler);
}

// Consumes the current token. Emits an error if it is not a newline. Then
// discards any duplicate newlines following it.
static void consumeLine(Compiler* compiler, const char* errorMessage)
{
  consume(compiler, TOKEN_LINE, errorMessage);
  ignoreNewlines(compiler);
}

// Variables and scopes --------------------------------------------------------

// Emits one bytecode instruction or single-byte argument. Returns its index.
static int emit(Compiler* compiler, int byte)
{
  wrenByteBufferWrite(compiler->parser->vm, &compiler->bytecode, (uint8_t)byte);

  // Assume the instruction is associated with the most recently consumed token.
  wrenIntBufferWrite(compiler->parser->vm, &compiler->debugSourceLines,
      compiler->parser->previous.line);

  return compiler->bytecode.count - 1;
}

// Emits one 16-bit argument, which will be written big endian.
static void emitShort(Compiler* compiler, int arg)
{
  emit(compiler, (arg >> 8) & 0xff);
  emit(compiler, arg & 0xff);
}

// Emits one bytecode instruction followed by a 8-bit argument. Returns the
// index of the argument in the bytecode.
static int emitByteArg(Compiler* compiler, Code instruction, int arg)
{
  emit(compiler, instruction);
  return emit(compiler, arg);
}

// Emits one bytecode instruction followed by a 16-bit argument, which will be
// written big endian.
static void emitShortArg(Compiler* compiler, Code instruction, int arg)
{
  emit(compiler, instruction);
  emitShort(compiler, arg);
}

// Emits [instruction] followed by a placeholder for a jump offset. The
// placeholder can be patched by calling [jumpPatch]. Returns the index of the
// placeholder.
static int emitJump(Compiler* compiler, Code instruction)
{
  emit(compiler, instruction);
  emit(compiler, 0xff);
  return emit(compiler, 0xff) - 1;
}

// Creates a new constant for the current value and emits the bytecode to load
// it from the constant table.
static void emitConstant(Compiler* compiler, Value value)
{
  int constant = addConstant(compiler, value);
  
  // Compile the code to load the constant.
  emitShortArg(compiler, CODE_CONSTANT, constant);
}

// Create a new local variable with [name]. Assumes the current scope is local
// and the name is unique.
static int addLocal(Compiler* compiler, const char* name, int length)
{
  Local* local = &compiler->locals[compiler->numLocals];
  local->name = name;
  local->length = length;
  local->depth = compiler->scopeDepth;
  local->isUpvalue = false;
  return compiler->numLocals++;
}

// Declares a variable in the current scope whose name is the given token.
//
// If [token] is `NULL`, uses the previously consumed token. Returns its symbol.
static int declareVariable(Compiler* compiler, Token* token)
{
  if (token == NULL) token = &compiler->parser->previous;

  if (token->length > MAX_VARIABLE_NAME)
  {
    error(compiler, "Variable name cannot be longer than %d characters.",
          MAX_VARIABLE_NAME);
  }

  // Top-level module scope.
  if (compiler->scopeDepth == -1)
  {
    int symbol = wrenDefineVariable(compiler->parser->vm,
                                    compiler->parser->module,
                                    token->start, token->length, NULL_VAL);

    if (symbol == -1)
    {
      error(compiler, "Module variable is already defined.");
    }
    else if (symbol == -2)
    {
      error(compiler, "Too many module variables defined.");
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

  return addLocal(compiler, token->start, token->length);
}

// Parses a name token and declares a variable in the current scope with that
// name. Returns its slot.
static int declareNamedVariable(Compiler* compiler)
{
  consume(compiler, TOKEN_NAME, "Expect variable name.");
  return declareVariable(compiler, NULL);
}

// Stores a variable with the previously defined symbol in the current scope.
static void defineVariable(Compiler* compiler, int symbol)
{
  // Store the variable. If it's a local, the result of the initializer is
  // in the correct slot on the stack already so we're done.
  if (compiler->scopeDepth >= 0) return;

  // It's a module-level variable, so store the value in the module slot and
  // then discard the temporary for the initializer.
  emitShortArg(compiler, CODE_STORE_MODULE_VAR, symbol);
  emit(compiler, CODE_POP);
}

// Starts a new local block scope.
static void pushScope(Compiler* compiler)
{
  compiler->scopeDepth++;
}

// Generates code to discard local variables at [depth] or greater. Does *not*
// actually undeclare variables or pop any scopes, though. This is called
// directly when compiling "break" statements to ditch the local variables
// before jumping out of the loop even though they are still in scope *past*
// the break instruction.
//
// Returns the number of local variables that were eliminated.
static int discardLocals(Compiler* compiler, int depth)
{
  ASSERT(compiler->scopeDepth > -1, "Cannot exit top-level scope.");

  int local = compiler->numLocals - 1;
  while (local >= 0 && compiler->locals[local].depth >= depth)
  {
    // If the local was closed over, make sure the upvalue gets closed when it
    // goes out of scope on the stack.
    if (compiler->locals[local].isUpvalue)
    {
      emit(compiler, CODE_CLOSE_UPVALUE);
    }
    else
    {
      emit(compiler, CODE_POP);
    }

    local--;
  }

  return compiler->numLocals - local - 1;
}

// Closes the last pushed block scope and discards any local variables declared
// in that scope. This should only be called in a statement context where no
// temporaries are still on the stack.
static void popScope(Compiler* compiler)
{
  compiler->numLocals -= discardLocals(compiler, compiler->scopeDepth);
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
//
// If it reaches a method boundary, this stops and returns -1 since methods do
// not close over local variables.
static int findUpvalue(Compiler* compiler, const char* name, int length)
{
  // If we are at the top level, we didn't find it.
  if (compiler->parent == NULL) return -1;
  
  // If we hit the method boundary (and the name isn't a static field), then
  // stop looking for it. We'll instead treat it as a self send.
  if (name[0] != '_' && compiler->parent->enclosingClass != NULL) return -1;
  
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
  // words, if it's a local variable in a non-immediately enclosing function.
  // This "flattens" closures automatically: it adds upvalues to all of the
  // intermediate functions to get from the function where a local is declared
  // all the way into the possibly deeply nested function that is closing over
  // it.
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
// the index of the name either in local scope, or the enclosing function's
// upvalue list. Does not search the module scope. Returns -1 if not found.
//
// Sets [loadInstruction] to the instruction needed to load the variable. Will
// be [CODE_LOAD_LOCAL] or [CODE_LOAD_UPVALUE].
static int resolveNonmodule(Compiler* compiler, const char* name, int length,
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
  return findUpvalue(compiler, name, length);
}

// Look up [name] in the current scope to see what name it is bound to. Returns
// the index of the name either in module scope, local scope, or the enclosing
// function's upvalue list. Returns -1 if not found.
//
// Sets [loadInstruction] to the instruction needed to load the variable. Will
// be one of [CODE_LOAD_LOCAL], [CODE_LOAD_UPVALUE], or [CODE_LOAD_MODULE_VAR].
static int resolveName(Compiler* compiler, const char* name, int length,
                       Code* loadInstruction)
{
  int nonmodule = resolveNonmodule(compiler, name, length, loadInstruction);
  if (nonmodule != -1) return nonmodule;

  *loadInstruction = CODE_LOAD_MODULE_VAR;
  return wrenSymbolTableFind(&compiler->parser->module->variableNames,
                             name, length);
}

static void loadLocal(Compiler* compiler, int slot)
{
  if (slot <= 8)
  {
    emit(compiler, CODE_LOAD_LOCAL_0 + slot);
    return;
  }

  emitByteArg(compiler, CODE_LOAD_LOCAL, slot);
}

// Discards memory owned by [compiler].
static void freeCompiler(Compiler* compiler)
{
  wrenByteBufferClear(compiler->parser->vm, &compiler->bytecode);
  wrenIntBufferClear(compiler->parser->vm, &compiler->debugSourceLines);
  wrenValueBufferClear(compiler->parser->vm, &compiler->constants);
}

// Finishes [compiler], which is compiling a function, method, or chunk of top
// level code. If there is a parent compiler, then this emits code in the
// parent compiler to load the resulting function.
static ObjFn* endCompiler(Compiler* compiler,
                          const char* debugName, int debugNameLength)
{
  // If we hit an error, don't bother creating the function since it's borked
  // anyway.
  if (compiler->parser->hasError)
  {
    wrenSetCompiler(compiler->parser->vm, compiler->parent);

    // Free the code and constants since they won't be used.
    freeCompiler(compiler);
    return NULL;
  }

  // Mark the end of the bytecode. Since it may contain multiple early returns,
  // we can't rely on CODE_RETURN to tell us we're at the end.
  emit(compiler, CODE_END);

  // Create a function object for the code we just compiled.
  ObjFn* fn = wrenNewFunction(compiler->parser->vm,
                              compiler->parser->module,
                              compiler->constants.data,
                              compiler->constants.count,
                              compiler->numUpvalues,
                              compiler->numParams,
                              compiler->bytecode.data,
                              compiler->bytecode.count,
                              debugName, debugNameLength,
                              compiler->debugSourceLines.data);
  wrenPushRoot(compiler->parser->vm, (Obj*)fn);

  // Clear constants. The constants are copied by wrenNewFunction
  wrenValueBufferClear(compiler->parser->vm, &compiler->constants);

  // In the function that contains this one, load the resulting function object.
  if (compiler->parent != NULL)
  {
    int constant = addConstant(compiler->parent, OBJ_VAL(fn));

    // If the function has no upvalues, we don't need to create a closure.
    // We can just load and run the function directly.
    if (compiler->numUpvalues == 0)
    {
      emitShortArg(compiler->parent, CODE_CONSTANT, constant);
    }
    else
    {
      // Capture the upvalues in the new closure object.
      emitShortArg(compiler->parent, CODE_CLOSURE, constant);

      // Emit arguments for each upvalue to know whether to capture a local or
      // an upvalue.
      for (int i = 0; i < compiler->numUpvalues; i++)
      {
        emit(compiler->parent, compiler->upvalues[i].isLocal ? 1 : 0);
        emit(compiler->parent, compiler->upvalues[i].index);
      }
    }
  }

  // Pop this compiler off the stack.
  wrenSetCompiler(compiler->parser->vm, compiler->parent);

  wrenPopRoot(compiler->parser->vm);

  #if WREN_DEBUG_DUMP_COMPILED_CODE
    wrenDumpCode(compiler->parser->vm, fn);
  #endif

  return fn;
}

// Grammar ---------------------------------------------------------------------

typedef enum
{
  PREC_NONE,
  PREC_LOWEST,
  PREC_ASSIGNMENT,    // =
  PREC_TERNARY,       // ?:
  PREC_LOGICAL_OR,    // ||
  PREC_LOGICAL_AND,   // &&
  PREC_EQUALITY,      // == !=
  PREC_IS,            // is
  PREC_COMPARISON,    // < > <= >=
  PREC_BITWISE_OR,    // |
  PREC_BITWISE_XOR,   // ^
  PREC_BITWISE_AND,   // &
  PREC_BITWISE_SHIFT, // << >>
  PREC_RANGE,         // .. ...
  PREC_TERM,          // + -
  PREC_FACTOR,        // * / %
  PREC_UNARY,         // unary - ! ~
  PREC_CALL,          // . () []
  PREC_PRIMARY
} Precedence;

typedef void (*GrammarFn)(Compiler*, bool allowAssignment);

typedef void (*SignatureFn)(Compiler* compiler, Signature* signature);

typedef struct
{
  GrammarFn prefix;
  GrammarFn infix;
  SignatureFn method;
  Precedence precedence;
  const char* name;
} GrammarRule;

// Forward declarations since the grammar is recursive.
static GrammarRule* getRule(TokenType type);
static void expression(Compiler* compiler);
static void statement(Compiler* compiler);
static void definition(Compiler* compiler);
static void parsePrecedence(Compiler* compiler, bool allowAssignment,
                            Precedence precedence);

// Replaces the placeholder argument for a previous CODE_JUMP or CODE_JUMP_IF
// instruction with an offset that jumps to the current end of bytecode.
static void patchJump(Compiler* compiler, int offset)
{
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = compiler->bytecode.count - offset - 2;
  // TODO: Check for overflow.
  compiler->bytecode.data[offset] = (jump >> 8) & 0xff;
  compiler->bytecode.data[offset + 1] = jump & 0xff;
}

// Parses a block body, after the initial "{" has been consumed.
//
// Returns true if it was a expression body, false if it was a statement body.
// (More precisely, returns true if a value was left on the stack. An empty
// block returns false.)
static bool finishBlock(Compiler* compiler)
{
  // Empty blocks do nothing.
  if (match(compiler, TOKEN_RIGHT_BRACE)) {
    return false;
  }

  // If there's no line after the "{", it's a single-expression body.
  if (!matchLine(compiler))
  {
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' at end of block.");
    return true;
  }

  // Empty blocks (with just a newline inside) do nothing.
  if (match(compiler, TOKEN_RIGHT_BRACE)) {
    return false;
  }

  // Compile the definition list.
  do
  {
    definition(compiler);

    // If we got into a weird error state, don't get stuck in a loop.
    if (peek(compiler) == TOKEN_EOF) return true;

    consumeLine(compiler, "Expect newline after statement.");
  }
  while (!match(compiler, TOKEN_RIGHT_BRACE));
  return false;
}

// Parses a method or function body, after the initial "{" has been consumed.
//
// It [isInitializer] is `true`, this is the body of a constructor initializer.
// In that case, this adds the code to ensure it returns `this`.
static void finishBody(Compiler* compiler, bool isInitializer)
{
  bool isExpressionBody = finishBlock(compiler);

  if (isInitializer)
  {
    // If the initializer body evaluates to a value, discard it.
    if (isExpressionBody) emit(compiler, CODE_POP);

    // The receiver is always stored in the first local slot.
    emit(compiler, CODE_LOAD_LOCAL_0);
  }
  else if (!isExpressionBody)
  {
    // Implicitly return null in statement bodies.
    emit(compiler, CODE_NULL);
  }

  emit(compiler, CODE_RETURN);
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

// Parses the rest of a comma-separated parameter list after the opening
// delimeter. Updates `arity` in [signature] with the number of parameters.
static void finishParameterList(Compiler* compiler, Signature* signature)
{
  do
  {
    ignoreNewlines(compiler);
    validateNumParameters(compiler, ++signature->arity);

    // Define a local variable in the method for the parameter.
    declareNamedVariable(compiler);
  }
  while (match(compiler, TOKEN_COMMA));
}

// Gets the symbol for a method [name] with [length].
static int methodSymbol(Compiler* compiler, const char* name, int length)
{
  return wrenSymbolTableEnsure(compiler->parser->vm,
      &compiler->parser->vm->methodNames, name, length);
}

// Appends characters to [name] (and updates [length]) for [numParams] "_"
// surrounded by [leftBracket] and [rightBracket].
static void signatureParameterList(char name[MAX_METHOD_SIGNATURE], int* length,
                                   int numParams, char leftBracket, char rightBracket)
{
  name[(*length)++] = leftBracket;
  for (int i = 0; i < numParams; i++)
  {
    if (i > 0) name[(*length)++] = ',';
    name[(*length)++] = '_';
  }
  name[(*length)++] = rightBracket;
}

// Fills [name] with the stringified version of [signature] and updates
// [length] to the resulting length.
static void signatureToString(Signature* signature,
                              char name[MAX_METHOD_SIGNATURE], int* length)
{
  *length = 0;

  // Build the full name from the signature.
  memcpy(name + *length, signature->name, signature->length);
  *length += signature->length;

  switch (signature->type)
  {
    case SIG_METHOD:
      signatureParameterList(name, length, signature->arity, '(', ')');
      break;

    case SIG_GETTER:
      // The signature is just the name.
      break;

    case SIG_SETTER:
      name[(*length)++] = '=';
      signatureParameterList(name, length, 1, '(', ')');
      break;

    case SIG_SUBSCRIPT:
      signatureParameterList(name, length, signature->arity, '[', ']');
      break;

    case SIG_SUBSCRIPT_SETTER:
      signatureParameterList(name, length, signature->arity - 1, '[', ']');
      name[(*length)++] = '=';
      signatureParameterList(name, length, 1, '(', ')');
      break;
      
    case SIG_INITIALIZER:
      memcpy(name, "init ", 5);
      memcpy(name + 5, signature->name, signature->length);
      *length = 5 + signature->length;
      signatureParameterList(name, length, signature->arity, '(', ')');
      break;
  }

  name[*length] = '\0';
}

// Gets the symbol for a method with [signature].
static int signatureSymbol(Compiler* compiler, Signature* signature)
{
  // Build the full name from the signature.
  char name[MAX_METHOD_SIGNATURE];
  int length;
  signatureToString(signature, name, &length);

  return methodSymbol(compiler, name, length);
}

// Returns a signature with [type] whose name is from the last consumed token.
static Signature signatureFromToken(Compiler* compiler, SignatureType type)
{
  Signature signature;
  
  // Get the token for the method name.
  Token* token = &compiler->parser->previous;
  signature.name = token->start;
  signature.length = token->length;
  signature.type = type;
  signature.arity = 0;

  if (signature.length > MAX_METHOD_NAME)
  {
    error(compiler, "Method names cannot be longer than %d characters.",
          MAX_METHOD_NAME);
    signature.length = MAX_METHOD_NAME;
  }
  
  return signature;
}

// Parses a comma-separated list of arguments. Modifies [signature] to include
// the arity of the argument list.
static void finishArgumentList(Compiler* compiler, Signature* signature)
{
  do
  {
    ignoreNewlines(compiler);
    validateNumParameters(compiler, ++signature->arity);
    expression(compiler);
  }
  while (match(compiler, TOKEN_COMMA));

  // Allow a newline before the closing delimiter.
  ignoreNewlines(compiler);
}

// Compiles a method call with [signature] using [instruction].
static void callSignature(Compiler* compiler, Code instruction,
                          Signature* signature)
{
  int symbol = signatureSymbol(compiler, signature);
  emitShortArg(compiler, (Code)(instruction + signature->arity), symbol);

  if (instruction == CODE_SUPER_0)
  {
    // Super calls need to be statically bound to the class's superclass. This
    // ensures we call the right method even when a method containing a super
    // call is inherited by another subclass.
    //
    // We bind it at class definition time by storing a reference to the
    // superclass in a constant. So, here, we create a slot in the constant
    // table and store NULL in it. When the method is bound, we'll look up the
    // superclass then and store it in the constant slot.
    emitShort(compiler, addConstant(compiler, NULL_VAL));
  }
}

// Compiles a method call with [numArgs] for a method with [name] with [length].
static void callMethod(Compiler* compiler, int numArgs, const char* name,
                       int length)
{
  int symbol = methodSymbol(compiler, name, length);
  emitShortArg(compiler, (Code)(CODE_CALL_0 + numArgs), symbol);
}

// Compiles an (optional) argument list for a method call with [methodSignature]
// and then calls it.
static void methodCall(Compiler* compiler, Code instruction,
                       Signature* signature)
{
  // Make a new signature that contains the updated arity and type based on
  // the arguments we find.
  Signature called = { signature->name, signature->length, SIG_GETTER, 0 };

  // Parse the argument list, if any.
  if (match(compiler, TOKEN_LEFT_PAREN))
  {
    called.type = SIG_METHOD;

    // Allow empty an argument list.
    if (peek(compiler) != TOKEN_RIGHT_PAREN)
    {
      finishArgumentList(compiler, &called);
    }
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  }

  // Parse the block argument, if any.
  if (match(compiler, TOKEN_LEFT_BRACE))
  {
    // Include the block argument in the arity.
    called.type = SIG_METHOD;
    called.arity++;

    Compiler fnCompiler;
    initCompiler(&fnCompiler, compiler->parser, compiler, true);

    // Make a dummy signature to track the arity.
    Signature fnSignature = { "", 0, SIG_METHOD, 0 };

    // Parse the parameter list, if any.
    if (match(compiler, TOKEN_PIPE))
    {
      finishParameterList(&fnCompiler, &fnSignature);
      consume(compiler, TOKEN_PIPE, "Expect '|' after function parameters.");
    }

    fnCompiler.numParams = fnSignature.arity;

    finishBody(&fnCompiler, false);

    // Name the function based on the method its passed to.
    char blockName[MAX_METHOD_SIGNATURE + 15];
    int blockLength;
    signatureToString(&called, blockName, &blockLength);
    memmove(blockName + blockLength, " block argument", 16);

    endCompiler(&fnCompiler, blockName, blockLength + 15);
  }

  // TODO: Allow Grace-style mixfix methods?

  // If this is a super() call for an initializer, make sure we got an actual
  // argument list.
  if (signature->type == SIG_INITIALIZER)
  {
    if (called.type != SIG_METHOD)
    {
      error(compiler, "A superclass constructor must have an argument list.");
    }
    
    called.type = SIG_INITIALIZER;
  }
  
  callSignature(compiler, instruction, &called);
}

// Compiles a call whose name is the previously consumed token. This includes
// getters, method calls with arguments, and setter calls.
static void namedCall(Compiler* compiler, bool allowAssignment,
                      Code instruction)
{
  // Get the token for the method name.
  Signature signature = signatureFromToken(compiler, SIG_GETTER);

  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    ignoreNewlines(compiler);

    // Build the setter signature.
    signature.type = SIG_SETTER;
    signature.arity = 1;

    // Compile the assigned value.
    expression(compiler);
    callSignature(compiler, instruction, &signature);
  }
  else
  {
    methodCall(compiler, instruction, &signature);
  }
}

// Loads the receiver of the currently enclosing method. Correctly handles
// functions defined inside methods.
static void loadThis(Compiler* compiler)
{
  Code loadInstruction;
  int index = resolveNonmodule(compiler, "this", 4, &loadInstruction);
  if (loadInstruction == CODE_LOAD_LOCAL)
  {
    loadLocal(compiler, index);
  }
  else
  {
    emitByteArg(compiler, loadInstruction, index);
  }
}

// Pushes the value for a module-level variable implicitly imported from core.
static void loadCoreVariable(Compiler* compiler, const char* name)
{
  int symbol = wrenSymbolTableFind(&compiler->parser->module->variableNames,
                                   name, strlen(name));
  ASSERT(symbol != -1, "Should have already defined core name.");
  emitShortArg(compiler, CODE_LOAD_MODULE_VAR, symbol);
}

// A parenthesized expression.
static void grouping(Compiler* compiler, bool allowAssignment)
{
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void call(Compiler* compiler, bool allowAssignment)
{
  // An infix parenthesized call is syntax sugar for invoking the "call" method
  // on the left-hand side.
  Signature signature = { "call", 4, SIG_METHOD, 0 };
  
  // Allow empty an argument list.
  if (peek(compiler) != TOKEN_RIGHT_PAREN)
  {
    // Parse the argument list.
    finishArgumentList(compiler, &signature);
  }
  
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
  
  // TODO: Allow block argument?
  
  callSignature(compiler, CODE_CALL_0, &signature);
}

// A list literal.
static void list(Compiler* compiler, bool allowAssignment)
{
  // Instantiate a new list.
  loadCoreVariable(compiler, "List");
  callMethod(compiler, 0, "new()", 5);
  
  // Compile the list elements. Each one compiles to a ".add()" call.
  do
  {
    ignoreNewlines(compiler);

    // Stop if we hit the end of the list.
    if (peek(compiler) == TOKEN_RIGHT_BRACKET) break;

    // The element.
    expression(compiler);
    callMethod(compiler, 1, "addCore_(_)", 11);
  } while (match(compiler, TOKEN_COMMA));

  // Allow newlines before the closing ']'.
  ignoreNewlines(compiler);
  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after list elements.");
}

// A map literal.
static void map(Compiler* compiler, bool allowAssignment)
{
  // Instantiate a new map.
  loadCoreVariable(compiler, "Map");
  callMethod(compiler, 0, "new()", 5);

  // Compile the map elements. Each one is compiled to just invoke the
  // subscript setter on the map.
  do
  {
    ignoreNewlines(compiler);

    // Stop if we hit the end of the map.
    if (peek(compiler) == TOKEN_RIGHT_BRACE) break;

    // The key.
    parsePrecedence(compiler, false, PREC_PRIMARY);
    consume(compiler, TOKEN_COLON, "Expect ':' after map key.");
    ignoreNewlines(compiler);

    // The value.
    expression(compiler);
    callMethod(compiler, 2, "addCore_(_,_)", 13);
  } while (match(compiler, TOKEN_COMMA));

  // Allow newlines before the closing '}'.
  ignoreNewlines(compiler);
  consume(compiler, TOKEN_RIGHT_BRACE, "Expect '}' after map entries.");
}

// Unary operators like `-foo`.
static void unaryOp(Compiler* compiler, bool allowAssignment)
{
  GrammarRule* rule = getRule(compiler->parser->previous.type);

  ignoreNewlines(compiler);

  // Compile the argument.
  parsePrecedence(compiler, false, (Precedence)(PREC_UNARY + 1));

  // Call the operator method on the left-hand side.
  callMethod(compiler, 0, rule->name, 1);
}

static void boolean(Compiler* compiler, bool allowAssignment)
{
  emit(compiler,
       compiler->parser->previous.type == TOKEN_FALSE ? CODE_FALSE : CODE_TRUE);
}

// Walks the compiler chain to find the compiler for the nearest class
// enclosing this one. Returns NULL if not currently inside a class definition.
static Compiler* getEnclosingClassCompiler(Compiler* compiler)
{
  while (compiler != NULL)
  {
    if (compiler->enclosingClass != NULL) return compiler;
    compiler = compiler->parent;
  }

  return NULL;
}

// Walks the compiler chain to find the nearest class enclosing this one.
// Returns NULL if not currently inside a class definition.
static ClassCompiler* getEnclosingClass(Compiler* compiler)
{
  compiler = getEnclosingClassCompiler(compiler);
  return compiler == NULL ? NULL : compiler->enclosingClass;
}

static void field(Compiler* compiler, bool allowAssignment)
{
  // Initialize it with a fake value so we can keep parsing and minimize the
  // number of cascaded errors.
  int field = 255;

  ClassCompiler* enclosingClass = getEnclosingClass(compiler);

  if (enclosingClass == NULL)
  {
    error(compiler, "Cannot reference a field outside of a class definition.");
  }
  else if (enclosingClass->isForeign)
  {
    error(compiler, "Cannot define fields in a foreign class.");
  }
  else if (enclosingClass->inStatic)
  {
    error(compiler, "Cannot use an instance field in a static method.");
  }
  else
  {
    // Look up the field, or implicitly define it.
    field = wrenSymbolTableEnsure(compiler->parser->vm, enclosingClass->fields,
        compiler->parser->previous.start,
        compiler->parser->previous.length);

    if (field >= MAX_FIELDS)
    {
      error(compiler, "A class can only have %d fields.", MAX_FIELDS);
    }
  }

  // If there's an "=" after a field name, it's an assignment.
  bool isLoad = true;
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    expression(compiler);
    isLoad = false;
  }

  // If we're directly inside a method, use a more optimal instruction.
  if (compiler->parent != NULL &&
      compiler->parent->enclosingClass == enclosingClass)
  {
    emitByteArg(compiler, isLoad ? CODE_LOAD_FIELD_THIS : CODE_STORE_FIELD_THIS,
                field);
  }
  else
  {
    loadThis(compiler);
    emitByteArg(compiler, isLoad ? CODE_LOAD_FIELD : CODE_STORE_FIELD, field);
  }
}

// Compiles a read or assignment to a variable at [index] using
// [loadInstruction].
static void variable(Compiler* compiler, bool allowAssignment, int index,
                     Code loadInstruction)
{
  // If there's an "=" after a bare name, it's a variable assignment.
  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    // Compile the right-hand side.
    expression(compiler);

    // Emit the store instruction.
    switch (loadInstruction)
    {
      case CODE_LOAD_LOCAL:
        emitByteArg(compiler, CODE_STORE_LOCAL, index);
        break;
      case CODE_LOAD_UPVALUE:
        emitByteArg(compiler, CODE_STORE_UPVALUE, index);
        break;
      case CODE_LOAD_MODULE_VAR:
        emitShortArg(compiler, CODE_STORE_MODULE_VAR, index);
        break;
      default:
        UNREACHABLE();
    }
  }
  else if (loadInstruction == CODE_LOAD_MODULE_VAR)
  {
    emitShortArg(compiler, loadInstruction, index);
  }
  else if (loadInstruction == CODE_LOAD_LOCAL)
  {
    loadLocal(compiler, index);
  }
  else
  {
    emitByteArg(compiler, loadInstruction, index);
  }
}

static void staticField(Compiler* compiler, bool allowAssignment)
{
  Code loadInstruction = CODE_LOAD_LOCAL;
  int index = 255;

  Compiler* classCompiler = getEnclosingClassCompiler(compiler);
  if (classCompiler == NULL)
  {
    error(compiler, "Cannot use a static field outside of a class definition.");
  }
  else
  {
    // Look up the name in the scope chain.
    Token* token = &compiler->parser->previous;

    // If this is the first time we've seen this static field, implicitly
    // define it as a variable in the scope surrounding the class definition.
    if (resolveLocal(classCompiler, token->start, token->length) == -1)
    {
      int symbol = declareVariable(classCompiler, NULL);

      // Implicitly initialize it to null.
      emit(classCompiler, CODE_NULL);
      defineVariable(classCompiler, symbol);
    }

    // It definitely exists now, so resolve it properly. This is different from
    // the above resolveLocal() call because we may have already closed over it
    // as an upvalue.
    index = resolveName(compiler, token->start, token->length,
                        &loadInstruction);
  }

  variable(compiler, allowAssignment, index, loadInstruction);
}

// Returns `true` if [name] is a local variable name (starts with a lowercase
// letter).
static bool isLocalName(const char* name)
{
  return name[0] >= 'a' && name[0] <= 'z';
}

// Compiles a variable name or method call with an implicit receiver.
static void name(Compiler* compiler, bool allowAssignment)
{
  // Look for the name in the scope chain up to the nearest enclosing method.
  Token* token = &compiler->parser->previous;

  Code loadInstruction;
  int index = resolveNonmodule(compiler, token->start, token->length,
                               &loadInstruction);
  if (index != -1)
  {
    variable(compiler, allowAssignment, index, loadInstruction);
    return;
  }

  // If a parameter list follows a resolved name, then it will be treated as an
  // infix call operator on it. So "foo(arg)" gets parsed to "foo.call(arg)" if
  // foo resolves to a variable and "this.foo(arg)" if we're in a class.
  //
  // This makes the *semantics* slightly context-sensitive but keeps the syntax
  // context free: "foo(arg)" is always *meaningful* regardless of how "foo" is
  // resolved, it just means one of two things.

  // If we're inside a method and the name is lowercase, treat it as a method
  // on this.
  if (isLocalName(token->start) && getEnclosingClass(compiler) != NULL)
  {
    loadThis(compiler);
    namedCall(compiler, allowAssignment, CODE_CALL_0);
    return;
  }

  // Otherwise, look for a module-level variable with the name.
  int module = wrenSymbolTableFind(&compiler->parser->module->variableNames,
                                   token->start, token->length);
  if (module == -1)
  {
    // If it's not a self-send or a closure, implicitly define a module-level
    // variable in the hopes that we get a real definition later.
    module = wrenDeclareVariable(compiler->parser->vm, compiler->parser->module,
                                 token->start, token->length);

    if (module == -2)
    {
      error(compiler, "Too many module variables defined.");
    }
  }

  variable(compiler, allowAssignment, module, CODE_LOAD_MODULE_VAR);
}

static void null(Compiler* compiler, bool allowAssignment)
{
  emit(compiler, CODE_NULL);
}

// A number or string literal.
static void literal(Compiler* compiler, bool allowAssignment)
{
  emitConstant(compiler, compiler->parser->previous.value);
}

// A string literal that contains interpolated expressions.
//
// Interpolation is syntactic sugar for calling ".join()" on a list. So the
// string:
//
//     "a %(b + c) d"
//
// is compiled roughly like:
//
//     ["a ", b + c, " d"].join()
static void stringInterpolation(Compiler* compiler, bool allowAssignment)
{
  // Instantiate a new list.
  loadCoreVariable(compiler, "List");
  callMethod(compiler, 0, "new()", 5);
  
  do
  {
    // The opening string part.
    literal(compiler, false);
    callMethod(compiler, 1, "addCore_(_)", 11);
    
    // The interpolated expression.
    ignoreNewlines(compiler);
    expression(compiler);
    callMethod(compiler, 1, "addCore_(_)", 11);
    
    ignoreNewlines(compiler);
  } while (match(compiler, TOKEN_INTERPOLATION));
  
  // The trailing string part.
  consume(compiler, TOKEN_STRING, "Expect end of string interpolation.");
  literal(compiler, false);
  callMethod(compiler, 1, "addCore_(_)", 11);
  
  // The list of interpolated parts.
  callMethod(compiler, 0, "join()", 6);
}

static void super_(Compiler* compiler, bool allowAssignment)
{
  ClassCompiler* enclosingClass = getEnclosingClass(compiler);

  if (enclosingClass == NULL)
  {
    error(compiler, "Cannot use 'super' outside of a method.");
  }

  loadThis(compiler);

  // TODO: Super operator calls.
  // TODO: There's no syntax for invoking a superclass constructor with a
  // different name from the enclosing one. Figure that out.

  // See if it's a named super call, or an unnamed one.
  if (match(compiler, TOKEN_DOT))
  {
    // Compile the superclass call.
    consume(compiler, TOKEN_NAME, "Expect method name after 'super.'.");
    namedCall(compiler, allowAssignment, CODE_SUPER_0);
  }
  else if (enclosingClass != NULL)
  {
    // No explicit name, so use the name of the enclosing method. Make sure we
    // check that enclosingClass isn't NULL first. We've already reported the
    // error, but we don't want to crash here.
    methodCall(compiler, CODE_SUPER_0, enclosingClass->signature);
  }
}

static void this_(Compiler* compiler, bool allowAssignment)
{
  if (getEnclosingClass(compiler) == NULL)
  {
    error(compiler, "Cannot use 'this' outside of a method.");
    return;
  }

  loadThis(compiler);
}

// Subscript or "array indexing" operator like `foo[bar]`.
static void subscript(Compiler* compiler, bool allowAssignment)
{
  Signature signature = { "", 0, SIG_SUBSCRIPT, 0 };

  // Parse the argument list.
  finishArgumentList(compiler, &signature);
  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after arguments.");

  if (match(compiler, TOKEN_EQ))
  {
    if (!allowAssignment) error(compiler, "Invalid assignment.");

    signature.type = SIG_SUBSCRIPT_SETTER;

    // Compile the assigned value.
    validateNumParameters(compiler, ++signature.arity);
    expression(compiler);
  }

  callSignature(compiler, CODE_CALL_0, &signature);
}

static void dot(Compiler* compiler, bool allowAssignment)
{
  ignoreNewlines(compiler);
  consume(compiler, TOKEN_NAME, "Expect method name after '.'.");
  namedCall(compiler, allowAssignment, CODE_CALL_0);
}

static void and_(Compiler* compiler, bool allowAssignment)
{
  ignoreNewlines(compiler);

  // Skip the right argument if the left is false.
  int jump = emitJump(compiler, CODE_AND);
  parsePrecedence(compiler, false, PREC_LOGICAL_AND);
  patchJump(compiler, jump);
}

static void or_(Compiler* compiler, bool allowAssignment)
{
  ignoreNewlines(compiler);

  // Skip the right argument if the left is true.
  int jump = emitJump(compiler, CODE_OR);
  parsePrecedence(compiler, false, PREC_LOGICAL_OR);
  patchJump(compiler, jump);
}

static void conditional(Compiler* compiler, bool allowAssignment)
{
  // Ignore newline after '?'.
  ignoreNewlines(compiler);

  // Jump to the else branch if the condition is false.
  int ifJump = emitJump(compiler, CODE_JUMP_IF);

  // Compile the then branch.
  parsePrecedence(compiler, allowAssignment, PREC_TERNARY);

  consume(compiler, TOKEN_COLON,
          "Expect ':' after then branch of conditional operator.");
  ignoreNewlines(compiler);

  // Jump over the else branch when the if branch is taken.
  int elseJump = emitJump(compiler, CODE_JUMP);

  // Compile the else branch.
  patchJump(compiler, ifJump);

  parsePrecedence(compiler, allowAssignment, PREC_ASSIGNMENT);

  // Patch the jump over the else.
  patchJump(compiler, elseJump);
}

void infixOp(Compiler* compiler, bool allowAssignment)
{
  GrammarRule* rule = getRule(compiler->parser->previous.type);

  // An infix operator cannot end an expression.
  ignoreNewlines(compiler);

  // Compile the right-hand side.
  parsePrecedence(compiler, false, (Precedence)(rule->precedence + 1));

  // Call the operator method on the left-hand side.
  Signature signature = { rule->name, (int)strlen(rule->name), SIG_METHOD, 1 };
  callSignature(compiler, CODE_CALL_0, &signature);
}

// Compiles a method signature for an infix operator.
void infixSignature(Compiler* compiler, Signature* signature)
{
  // Add the RHS parameter.
  signature->type = SIG_METHOD;
  signature->arity = 1;

  // Parse the parameter name.
  consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after operator name.");
  declareNamedVariable(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameter name.");
}

// Compiles a method signature for an unary operator (i.e. "!").
void unarySignature(Compiler* compiler, Signature* signature)
{
  // Do nothing. The name is already complete.
  signature->type = SIG_GETTER;
}

// Compiles a method signature for an operator that can either be unary or
// infix (i.e. "-").
void mixedSignature(Compiler* compiler, Signature* signature)
{
  signature->type = SIG_GETTER;

  // If there is a parameter, it's an infix operator, otherwise it's unary.
  if (match(compiler, TOKEN_LEFT_PAREN))
  {
    // Add the RHS parameter.
    signature->type = SIG_METHOD;
    signature->arity = 1;

    // Parse the parameter name.
    declareNamedVariable(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameter name.");
  }
}

// Compiles an optional setter parameter in a method [signature].
//
// Returns `true` if it was a setter.
static bool maybeSetter(Compiler* compiler, Signature* signature)
{
  // See if it's a setter.
  if (!match(compiler, TOKEN_EQ)) return false;

  // It's a setter.
  if (signature->type == SIG_SUBSCRIPT)
  {
    signature->type = SIG_SUBSCRIPT_SETTER;
  }
  else
  {
    signature->type = SIG_SETTER;
  }

  // Parse the value parameter.
  consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after '='.");
  declareNamedVariable(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameter name.");

  signature->arity++;

  return true;
}

// Compiles a method signature for a subscript operator.
void subscriptSignature(Compiler* compiler, Signature* signature)
{
  signature->type = SIG_SUBSCRIPT;

  // The signature currently has "[" as its name since that was the token that
  // matched it. Clear that out.
  signature->length = 0;

  // Parse the parameters inside the subscript.
  finishParameterList(compiler, signature);
  consume(compiler, TOKEN_RIGHT_BRACKET, "Expect ']' after parameters.");

  maybeSetter(compiler, signature);
}

// Parses an optional parenthesized parameter list. Updates `type` and `arity`
// in [signature] to match what was parsed.
static void parameterList(Compiler* compiler, Signature* signature)
{
  // The parameter list is optional.
  if (!match(compiler, TOKEN_LEFT_PAREN)) return;
  
  signature->type = SIG_METHOD;
  
  // Allow an empty parameter list.
  if (match(compiler, TOKEN_RIGHT_PAREN)) return;

  finishParameterList(compiler, signature);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
}

// Compiles a method signature for a named method or setter.
void namedSignature(Compiler* compiler, Signature* signature)
{
  signature->type = SIG_GETTER;
  
  // If it's a setter, it can't also have a parameter list.
  if (maybeSetter(compiler, signature)) return;

  // Regular named method with an optional parameter list.
  parameterList(compiler, signature);
}

// Compiles a method signature for a constructor.
void constructorSignature(Compiler* compiler, Signature* signature)
{
  consume(compiler, TOKEN_NAME, "Expect constructor name after 'construct'.");
  
  // Capture the name.
  *signature = signatureFromToken(compiler, SIG_INITIALIZER);
  
  if (match(compiler, TOKEN_EQ))
  {
    error(compiler, "A constructor cannot be a setter.");
  }

  if (!match(compiler, TOKEN_LEFT_PAREN))
  {
    error(compiler, "A constructor cannot be a getter.");
    return;
  }
  
  // Allow an empty parameter list.
  if (match(compiler, TOKEN_RIGHT_PAREN)) return;
  
  finishParameterList(compiler, signature);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
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
  /* TOKEN_LEFT_PAREN    */ { grouping, call, NULL, PREC_CALL, NULL },
  /* TOKEN_RIGHT_PAREN   */ UNUSED,
  /* TOKEN_LEFT_BRACKET  */ { list, subscript, subscriptSignature, PREC_CALL, NULL },
  /* TOKEN_RIGHT_BRACKET */ UNUSED,
  /* TOKEN_LEFT_BRACE    */ PREFIX(map),
  /* TOKEN_RIGHT_BRACE   */ UNUSED,
  /* TOKEN_COLON         */ UNUSED,
  /* TOKEN_DOT           */ INFIX(PREC_CALL, dot),
  /* TOKEN_DOTDOT        */ INFIX_OPERATOR(PREC_RANGE, ".."),
  /* TOKEN_DOTDOTDOT     */ INFIX_OPERATOR(PREC_RANGE, "..."),
  /* TOKEN_COMMA         */ UNUSED,
  /* TOKEN_STAR          */ INFIX_OPERATOR(PREC_FACTOR, "*"),
  /* TOKEN_SLASH         */ INFIX_OPERATOR(PREC_FACTOR, "/"),
  /* TOKEN_PERCENT       */ INFIX_OPERATOR(PREC_FACTOR, "%"),
  /* TOKEN_PLUS          */ INFIX_OPERATOR(PREC_TERM, "+"),
  /* TOKEN_MINUS         */ OPERATOR("-"),
  /* TOKEN_LTLT          */ INFIX_OPERATOR(PREC_BITWISE_SHIFT, "<<"),
  /* TOKEN_GTGT          */ INFIX_OPERATOR(PREC_BITWISE_SHIFT, ">>"),
  /* TOKEN_PIPE          */ INFIX_OPERATOR(PREC_BITWISE_OR, "|"),
  /* TOKEN_PIPEPIPE      */ INFIX(PREC_LOGICAL_OR, or_),
  /* TOKEN_CARET         */ INFIX_OPERATOR(PREC_BITWISE_XOR, "^"),
  /* TOKEN_AMP           */ INFIX_OPERATOR(PREC_BITWISE_AND, "&"),
  /* TOKEN_AMPAMP        */ INFIX(PREC_LOGICAL_AND, and_),
  /* TOKEN_BANG          */ PREFIX_OPERATOR("!"),
  /* TOKEN_TILDE         */ PREFIX_OPERATOR("~"),
  /* TOKEN_QUESTION      */ INFIX(PREC_ASSIGNMENT, conditional),
  /* TOKEN_EQ            */ UNUSED,
  /* TOKEN_LT            */ INFIX_OPERATOR(PREC_COMPARISON, "<"),
  /* TOKEN_GT            */ INFIX_OPERATOR(PREC_COMPARISON, ">"),
  /* TOKEN_LTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, "<="),
  /* TOKEN_GTEQ          */ INFIX_OPERATOR(PREC_COMPARISON, ">="),
  /* TOKEN_EQEQ          */ INFIX_OPERATOR(PREC_EQUALITY, "=="),
  /* TOKEN_BANGEQ        */ INFIX_OPERATOR(PREC_EQUALITY, "!="),
  /* TOKEN_BREAK         */ UNUSED,
  /* TOKEN_CLASS         */ UNUSED,
  /* TOKEN_CONSTRUCT     */ { NULL, NULL, constructorSignature, PREC_NONE, NULL },
  /* TOKEN_ELSE          */ UNUSED,
  /* TOKEN_FALSE         */ PREFIX(boolean),
  /* TOKEN_FOR           */ UNUSED,
  /* TOKEN_FOREIGN       */ UNUSED,
  /* TOKEN_IF            */ UNUSED,
  /* TOKEN_IMPORT        */ UNUSED,
  /* TOKEN_IN            */ UNUSED,
  /* TOKEN_IS            */ INFIX_OPERATOR(PREC_IS, "is"),
  /* TOKEN_NULL          */ PREFIX(null),
  /* TOKEN_RETURN        */ UNUSED,
  /* TOKEN_STATIC        */ UNUSED,
  /* TOKEN_SUPER         */ PREFIX(super_),
  /* TOKEN_THIS          */ PREFIX(this_),
  /* TOKEN_TRUE          */ PREFIX(boolean),
  /* TOKEN_VAR           */ UNUSED,
  /* TOKEN_WHILE         */ UNUSED,
  /* TOKEN_FIELD         */ PREFIX(field),
  /* TOKEN_STATIC_FIELD  */ PREFIX(staticField),
  /* TOKEN_NAME          */ { name, NULL, namedSignature, PREC_NONE, NULL },
  /* TOKEN_NUMBER        */ PREFIX(literal),
  /* TOKEN_STRING        */ PREFIX(literal),
  /* TOKEN_INTERPOLATION */ PREFIX(stringInterpolation),
  /* TOKEN_LINE          */ UNUSED,
  /* TOKEN_ERROR         */ UNUSED,
  /* TOKEN_EOF           */ UNUSED
};

// Gets the [GrammarRule] associated with tokens of [type].
static GrammarRule* getRule(TokenType type)
{
  return &rules[type];
}

// The main entrypoint for the top-down operator precedence parser.
void parsePrecedence(Compiler* compiler, bool allowAssignment,
                     Precedence precedence)
{
  nextToken(compiler->parser);
  GrammarFn prefix = rules[compiler->parser->previous.type].prefix;

  if (prefix == NULL)
  {
    error(compiler, "Expected expression.");
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

// Parses a curly block or an expression statement. Used in places like the
// arms of an if statement where either a single expression or a curly body is
// allowed.
void block(Compiler* compiler)
{
  // Curly block.
  if (match(compiler, TOKEN_LEFT_BRACE))
  {
    pushScope(compiler);
    if (finishBlock(compiler))
    {
      // Block was an expression, so discard it.
      emit(compiler, CODE_POP);
    }
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
  Code instruction = (Code)bytecode[ip];
  switch (instruction)
  {
    case CODE_NULL:
    case CODE_FALSE:
    case CODE_TRUE:
    case CODE_POP:
    case CODE_DUP:
    case CODE_CLOSE_UPVALUE:
    case CODE_RETURN:
    case CODE_END:
    case CODE_LOAD_LOCAL_0:
    case CODE_LOAD_LOCAL_1:
    case CODE_LOAD_LOCAL_2:
    case CODE_LOAD_LOCAL_3:
    case CODE_LOAD_LOCAL_4:
    case CODE_LOAD_LOCAL_5:
    case CODE_LOAD_LOCAL_6:
    case CODE_LOAD_LOCAL_7:
    case CODE_LOAD_LOCAL_8:
    case CODE_CONSTRUCT:
    case CODE_FOREIGN_CONSTRUCT:
    case CODE_FOREIGN_CLASS:
      return 0;

    case CODE_LOAD_LOCAL:
    case CODE_STORE_LOCAL:
    case CODE_LOAD_UPVALUE:
    case CODE_STORE_UPVALUE:
    case CODE_LOAD_FIELD_THIS:
    case CODE_STORE_FIELD_THIS:
    case CODE_LOAD_FIELD:
    case CODE_STORE_FIELD:
    case CODE_CLASS:
      return 1;

    case CODE_CONSTANT:
    case CODE_LOAD_MODULE_VAR:
    case CODE_STORE_MODULE_VAR:
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
    case CODE_JUMP:
    case CODE_LOOP:
    case CODE_JUMP_IF:
    case CODE_AND:
    case CODE_OR:
    case CODE_METHOD_INSTANCE:
    case CODE_METHOD_STATIC:
    case CODE_LOAD_MODULE:
      return 2;

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
    case CODE_IMPORT_VARIABLE:
      return 4;

    case CODE_CLOSURE:
    {
      int constant = (bytecode[ip + 1] << 8) | bytecode[ip + 2];
      ObjFn* loadedFn = AS_FN(constants[constant]);

      // There are two bytes for the constant, then two for each upvalue.
      return 2 + (loadedFn->numUpvalues * 2);
    }

    default:
      UNREACHABLE();
      return 0;
  }
}

// Marks the beginning of a loop. Keeps track of the current instruction so we
// know what to loop back to at the end of the body.
static void startLoop(Compiler* compiler, Loop* loop)
{
  loop->enclosing = compiler->loop;
  loop->start = compiler->bytecode.count - 1;
  loop->scopeDepth = compiler->scopeDepth;
  compiler->loop = loop;
}

// Emits the [CODE_JUMP_IF] instruction used to test the loop condition and
// potentially exit the loop. Keeps track of the instruction so we can patch it
// later once we know where the end of the body is.
static void testExitLoop(Compiler* compiler)
{
  compiler->loop->exitJump = emitJump(compiler, CODE_JUMP_IF);
}

// Compiles the body of the loop and tracks its extent so that contained "break"
// statements can be handled correctly.
static void loopBody(Compiler* compiler)
{
  compiler->loop->body = compiler->bytecode.count;
  block(compiler);
}

// Ends the current innermost loop. Patches up all jumps and breaks now that
// we know where the end of the loop is.
static void endLoop(Compiler* compiler)
{
  int loopOffset = compiler->bytecode.count - compiler->loop->start + 2;
  // TODO: Check for overflow.
  emitShortArg(compiler, CODE_LOOP, loopOffset);

  patchJump(compiler, compiler->loop->exitJump);

  // Find any break placeholder instructions (which will be CODE_END in the
  // bytecode) and replace them with real jumps.
  int i = compiler->loop->body;
  while (i < compiler->bytecode.count)
  {
    if (compiler->bytecode.data[i] == CODE_END)
    {
      compiler->bytecode.data[i] = CODE_JUMP;
      patchJump(compiler, i + 1);
      i += 3;
    }
    else
    {
      // Skip this instruction and its arguments.
      i += 1 + getNumArguments(compiler->bytecode.data,
                               compiler->constants.data, i);
    }
  }

  compiler->loop = compiler->loop->enclosing;
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
  //       while (iter_ = seq_.iterate(iter_)) {
  //         var i = seq_.iteratorValue(iter_)
  //         IO.write(i)
  //       }
  //     }
  //
  // It's not exactly this, because the synthetic variables `seq_` and `iter_`
  // actually get names that aren't valid Wren identfiers, but that's the basic
  // idea.
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
  ignoreNewlines(compiler);

  // Evaluate the sequence expression and store it in a hidden local variable.
  // The space in the variable name ensures it won't collide with a user-defined
  // variable.
  expression(compiler);
  int seqSlot = addLocal(compiler, "seq ", 4);

  // Create another hidden local for the iterator object.
  null(compiler, false);
  int iterSlot = addLocal(compiler, "iter ", 5);

  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after loop expression.");

  Loop loop;
  startLoop(compiler, &loop);

  // Advance the iterator by calling the ".iterate" method on the sequence.
  loadLocal(compiler, seqSlot);
  loadLocal(compiler, iterSlot);

  callMethod(compiler, 1, "iterate(_)", 10);

  // Store the iterator back in its local for the next iteration.
  emitByteArg(compiler, CODE_STORE_LOCAL, iterSlot);
  // TODO: We can probably get this working with a bit less stack juggling.

  testExitLoop(compiler);

  // Get the current value in the sequence by calling ".iteratorValue".
  loadLocal(compiler, seqSlot);
  loadLocal(compiler, iterSlot);

  callMethod(compiler, 1, "iteratorValue(_)", 16);

  // Bind the loop variable in its own scope. This ensures we get a fresh
  // variable each iteration so that closures for it don't all see the same one.
  pushScope(compiler);
  addLocal(compiler, name, length);

  loopBody(compiler);

  // Loop variable.
  popScope(compiler);

  endLoop(compiler);

  // Hidden variables.
  popScope(compiler);
}

static void whileStatement(Compiler* compiler)
{
  Loop loop;
  startLoop(compiler, &loop);

  // Compile the condition.
  consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
  expression(compiler);
  consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");

  testExitLoop(compiler);
  loopBody(compiler);
  endLoop(compiler);
}

// Compiles a statement. These can only appear at the top-level or within
// curly blocks. Unlike expressions, these do not leave a value on the stack.
void statement(Compiler* compiler)
{
  if (match(compiler, TOKEN_BREAK))
  {
    if (compiler->loop == NULL)
    {
      error(compiler, "Cannot use 'break' outside of a loop.");
      return;
    }

    // Since we will be jumping out of the scope, make sure any locals in it
    // are discarded first.
    discardLocals(compiler, compiler->loop->scopeDepth + 1);

    // Emit a placeholder instruction for the jump to the end of the body. When
    // we're done compiling the loop body and know where the end is, we'll
    // replace these with `CODE_JUMP` instructions with appropriate offsets.
    // We use `CODE_END` here because that can't occur in the middle of
    // bytecode.
    emitJump(compiler, CODE_END);
    return;
  }

  if (match(compiler, TOKEN_FOR)) {
    forStatement(compiler);
    return;
  }

  if (match(compiler, TOKEN_IF))
  {
    // Compile the condition.
    consume(compiler, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    expression(compiler);
    consume(compiler, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");

    // Jump to the else branch if the condition is false.
    int ifJump = emitJump(compiler, CODE_JUMP_IF);

    // Compile the then branch.
    block(compiler);

    // Compile the else branch if there is one.
    if (match(compiler, TOKEN_ELSE))
    {
      // Jump over the else branch when the if branch is taken.
      int elseJump = emitJump(compiler, CODE_JUMP);

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
    if (peek(compiler) == TOKEN_LINE)
    {
      // Implicitly return null if there is no value.
      emit(compiler, CODE_NULL);
    }
    else
    {
      expression(compiler);
    }

    emit(compiler, CODE_RETURN);
    return;
  }

  if (match(compiler, TOKEN_WHILE)) {
    whileStatement(compiler);
    return;
  }

  // Expression statement.
  expression(compiler);
  emit(compiler, CODE_POP);
}

// Creates a matching constructor method for an initializer with [signature]
// and [initializerSymbol].
//
// Construction is a two-stage process in Wren that involves two separate
// methods. There is a static method that allocates a new instance of the class.
// It then invokes an initializer method on the new instance, forwarding all of
// the constructor arguments to it.
//
// The allocator method always has a fixed implementation:
//
//     CODE_CONSTRUCT - Replace the class in slot 0 with a new instance of it.
//     CODE_CALL      - Invoke the initializer on the new instance.
//
// This creates that method and calls the initializer with [initializerSymbol].
static void createConstructor(Compiler* compiler, Signature* signature,
                              int initializerSymbol)
{
  Compiler methodCompiler;
  initCompiler(&methodCompiler, compiler->parser, compiler, false);
  
  // Allocate the instance.
  emit(&methodCompiler, compiler->enclosingClass->isForeign
       ? CODE_FOREIGN_CONSTRUCT : CODE_CONSTRUCT);
  
  // Run its initializer.
  emitShortArg(&methodCompiler, (Code)(CODE_CALL_0 + signature->arity),
               initializerSymbol);
  
  // Return the instance.
  emit(&methodCompiler, CODE_RETURN);
  
  endCompiler(&methodCompiler, "", 0);
}

// Loads the enclosing class onto the stack and then binds the function already
// on the stack as a method on that class.
static void defineMethod(Compiler* compiler, int classSlot, bool isStatic,
                         int methodSymbol)
{
  // Load the class. We have to do this for each method because we can't
  // keep the class on top of the stack. If there are static fields, they
  // will be locals above the initial variable slot for the class on the
  // stack. To skip past those, we just load the class each time right before
  // defining a method.
  if (compiler->scopeDepth == 0)
  {
    // The class is at the top level (scope depth is 0, not -1 to account for
    // the static variable scope surrounding the class itself), so load it from
    // there.
    emitShortArg(compiler, CODE_LOAD_MODULE_VAR, classSlot);
  }
  else
  {
    loadLocal(compiler, classSlot);
  }
  
  // Define the method.
  Code instruction = isStatic ? CODE_METHOD_STATIC : CODE_METHOD_INSTANCE;
  emitShortArg(compiler, instruction, methodSymbol);
}

// Compiles a method definition inside a class body.
//
// Returns `true` if it compiled successfully, or `false` if the method couldn't
// be parsed.
static bool method(Compiler* compiler, ClassCompiler* classCompiler,
                   int classSlot)
{
  // TODO: What about foreign constructors?
  bool isForeign = match(compiler, TOKEN_FOREIGN);
  classCompiler->inStatic = match(compiler, TOKEN_STATIC);
    
  SignatureFn signatureFn = rules[compiler->parser->current.type].method;
  nextToken(compiler->parser);
  
  if (signatureFn == NULL)
  {
    error(compiler, "Expect method definition.");
    return false;
  }
  
  // Build the method signature.
  Signature signature = signatureFromToken(compiler, SIG_GETTER);
  classCompiler->signature = &signature;

  Compiler methodCompiler;
  initCompiler(&methodCompiler, compiler->parser, compiler, false);

  // Compile the method signature.
  signatureFn(&methodCompiler, &signature);
  
  if (classCompiler->inStatic && signature.type == SIG_INITIALIZER)
  {
    error(compiler, "A constructor cannot be static.");
  }
  
  // Include the full signature in debug messages in stack traces.
  char fullSignature[MAX_METHOD_SIGNATURE];
  int length;
  signatureToString(&signature, fullSignature, &length);

  if (isForeign)
  {
    // Define a constant for the signature.
    emitConstant(compiler, wrenNewString(compiler->parser->vm,
                                         fullSignature, length));

    // We don't need the function we started compiling in the parameter list
    // any more.
    freeCompiler(&methodCompiler);
  }
  else
  {
    consume(compiler, TOKEN_LEFT_BRACE, "Expect '{' to begin method body.");
    finishBody(&methodCompiler, signature.type == SIG_INITIALIZER);
    endCompiler(&methodCompiler, fullSignature, length);
  }
  
  // Define the method. For a constructor, this defines the instance
  // initializer method.
  int methodSymbol = signatureSymbol(compiler, &signature);
  defineMethod(compiler, classSlot, classCompiler->inStatic, methodSymbol);

  if (signature.type == SIG_INITIALIZER)
  {
    // Also define a matching constructor method on the metaclass.
    signature.type = SIG_METHOD;
    int constructorSymbol = signatureSymbol(compiler, &signature);
    
    createConstructor(compiler, &signature, methodSymbol);
    defineMethod(compiler, classSlot, true, constructorSymbol);
  }

  return true;
}

// Compiles a class definition. Assumes the "class" token has already been
// consumed (along with a possibly preceding "foreign" token).
static void classDefinition(Compiler* compiler, bool isForeign)
{
  // Create a variable to store the class in.
  int slot = declareNamedVariable(compiler);

  // Make a string constant for the name.
  emitConstant(compiler, wrenNewString(compiler->parser->vm,
      compiler->parser->previous.start, compiler->parser->previous.length));

  // Load the superclass (if there is one).
  if (match(compiler, TOKEN_IS))
  {
    parsePrecedence(compiler, false, PREC_CALL);
  }
  else
  {
    // Implicitly inherit from Object.
    loadCoreVariable(compiler, "Object");
  }

  // Store a placeholder for the number of fields argument. We don't know
  // the value until we've compiled all the methods to see which fields are
  // used.
  int numFieldsInstruction = -1;
  if (isForeign)
  {
    emit(compiler, CODE_FOREIGN_CLASS);
  }
  else
  {
    numFieldsInstruction = emitByteArg(compiler, CODE_CLASS, 255);
  }

  // Store it in its name.
  defineVariable(compiler, slot);

  // Push a local variable scope. Static fields in a class body are hoisted out
  // into local variables declared in this scope. Methods that use them will
  // have upvalues referencing them.
  pushScope(compiler);

  ClassCompiler classCompiler;
  classCompiler.isForeign = isForeign;

  // Set up a symbol table for the class's fields. We'll initially compile
  // them to slots starting at zero. When the method is bound to the class, the
  // bytecode will be adjusted by [wrenBindMethod] to take inherited fields
  // into account.
  SymbolTable fields;
  wrenSymbolTableInit(&fields);

  classCompiler.fields = &fields;

  compiler->enclosingClass = &classCompiler;

  // Compile the method definitions.
  consume(compiler, TOKEN_LEFT_BRACE, "Expect '{' after class declaration.");
  matchLine(compiler);

  while (!match(compiler, TOKEN_RIGHT_BRACE))
  {
    if (!method(compiler, &classCompiler, slot)) break;
    
    // Don't require a newline after the last definition.
    if (match(compiler, TOKEN_RIGHT_BRACE)) break;

    consumeLine(compiler, "Expect newline after definition in class.");
  }
  
  // Update the class with the number of fields.
  if (!isForeign)
  {
    compiler->bytecode.data[numFieldsInstruction] = (uint8_t)fields.count;
  }
  
  wrenSymbolTableClear(compiler->parser->vm, &fields);

  compiler->enclosingClass = NULL;

  popScope(compiler);
}

// Compiles an "import" statement.
static void import(Compiler* compiler)
{
  consume(compiler, TOKEN_STRING, "Expect a string after 'import'.");
  int moduleConstant = addConstant(compiler, compiler->parser->previous.value);

  // Load the module.
  emitShortArg(compiler, CODE_LOAD_MODULE, moduleConstant);

  // Discard the unused result value from calling the module's fiber.
  emit(compiler, CODE_POP);

  // The for clause is optional.
  if (!match(compiler, TOKEN_FOR)) return;

  // Compile the comma-separated list of variables to import.
  do
  {
    int slot = declareNamedVariable(compiler);

    // Define a string constant for the variable name.
    int variableConstant = addConstant(compiler,
        wrenNewString(compiler->parser->vm,
                      compiler->parser->previous.start,
                      compiler->parser->previous.length));

    // Load the variable from the other module.
    emitShortArg(compiler, CODE_IMPORT_VARIABLE, moduleConstant);
    emitShort(compiler, variableConstant);

    // Store the result in the variable here.
    defineVariable(compiler, slot);
  } while (match(compiler, TOKEN_COMMA));
}

// Compiles a "var" variable definition statement.
static void variableDefinition(Compiler* compiler)
{
  // Grab its name, but don't declare it yet. A (local) variable shouldn't be
  // in scope in its own initializer.
  consume(compiler, TOKEN_NAME, "Expect variable name.");
  Token nameToken = compiler->parser->previous;

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

  // Now put it in scope.
  int symbol = declareVariable(compiler, &nameToken);
  defineVariable(compiler, symbol);
}

// Compiles a "definition". These are the statements that bind new variables.
// They can only appear at the top level of a block and are prohibited in places
// like the non-curly body of an if or while.
void definition(Compiler* compiler)
{
  if (match(compiler, TOKEN_CLASS))
  {
    classDefinition(compiler, false);
  }
  else if (match(compiler, TOKEN_FOREIGN))
  {
    consume(compiler, TOKEN_CLASS, "Expect 'class' after 'foreign'.");
    classDefinition(compiler, true);
  }
  else if (match(compiler, TOKEN_IMPORT))
  {
    import(compiler);
  }
  else if (match(compiler, TOKEN_VAR))
  {
    variableDefinition(compiler);
  }
  else
  {
    block(compiler);
  }
}

ObjFn* wrenCompile(WrenVM* vm, ObjModule* module, const char* source,
                   bool printErrors)
{
  Parser parser;
  parser.vm = vm;
  parser.module = module;
  parser.source = source;

  parser.tokenStart = source;
  parser.currentChar = source;
  parser.currentLine = 1;
  parser.numParens = 0;

  // Zero-init the current token. This will get copied to previous when
  // advance() is called below.
  parser.current.type = TOKEN_ERROR;
  parser.current.start = source;
  parser.current.length = 0;
  parser.current.line = 0;
  parser.current.value = UNDEFINED_VAL;

  // Ignore leading newlines.
  parser.skipNewlines = true;
  parser.printErrors = printErrors;
  parser.hasError = false;

  // Read the first token.
  nextToken(&parser);

  Compiler compiler;
  initCompiler(&compiler, &parser, NULL, true);
  ignoreNewlines(&compiler);

  while (!match(&compiler, TOKEN_EOF))
  {
    definition(&compiler);

    // If there is no newline, it must be the end of the block on the same line.
    if (!matchLine(&compiler))
    {
      consume(&compiler, TOKEN_EOF, "Expect end of file.");
      break;
    }
  }

  emit(&compiler, CODE_NULL);
  emit(&compiler, CODE_RETURN);

  // See if there are any implicitly declared module-level variables that never
  // got an explicit definition.
  // TODO: It would be nice if the error was on the line where it was used.
  for (int i = 0; i < parser.module->variables.count; i++)
  {
    if (IS_UNDEFINED(parser.module->variables.data[i]))
    {
      error(&compiler, "Variable '%s' is used but not defined.",
            parser.module->variableNames.data[i].buffer);
    }
  }
  
  return endCompiler(&compiler, "(script)", 8);
}

void wrenBindMethodCode(ObjClass* classObj, ObjFn* fn)
{
  int ip = 0;
  for (;;)
  {
    Code instruction = (Code)fn->bytecode[ip++];
    switch (instruction)
    {
      case CODE_LOAD_FIELD:
      case CODE_STORE_FIELD:
      case CODE_LOAD_FIELD_THIS:
      case CODE_STORE_FIELD_THIS:
        // Shift this class's fields down past the inherited ones. We don't
        // check for overflow here because we'll see if the number of fields
        // overflows when the subclass is created.
        fn->bytecode[ip++] += classObj->superclass->numFields;
        break;

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
      {
        // Skip over the symbol.
        ip += 2;
        
        // Fill in the constant slot with a reference to the superclass.
        int constant = (fn->bytecode[ip] << 8) | fn->bytecode[ip + 1];
        fn->constants[constant] = OBJ_VAL(classObj->superclass);
        break;
      }

      case CODE_CLOSURE:
      {
        // Bind the nested closure too.
        int constant = (fn->bytecode[ip] << 8) | fn->bytecode[ip + 1];
        wrenBindMethodCode(classObj, AS_FN(fn->constants[constant]));

        ip += getNumArguments(fn->bytecode, fn->constants, ip - 1);
        break;
      }

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
  wrenGrayValue(vm, compiler->parser->current.value);
  wrenGrayValue(vm, compiler->parser->previous.value);

  // Walk up the parent chain to mark the outer compilers too. The VM only
  // tracks the innermost one.
  do
  {
    wrenGrayBuffer(vm, &compiler->constants);
    compiler = compiler->parent;
  }
  while (compiler != NULL);
}
