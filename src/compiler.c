#include <stdio.h>
#include <stdarg.h>
#include "compiler.h"

typedef struct
{
  Buffer* source;
  Token* current;

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
static Token* match(Compiler* compiler, TokenType expected);
static Token* consume(Compiler* compiler, TokenType expected);
static Token* advance(Compiler* compiler);
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
  NULL, // TOKEN_EMBEDDED
  prefixLiteral, // TOKEN_NAME
  prefixLiteral, // TOKEN_NUMBER
  prefixLiteral, // TOKEN_STRING
  NULL, // TOKEN_LINE
  NULL, // TOKEN_WHITESPACE
  NULL, // TOKEN_INDENT
  NULL, // TOKEN_OUTDENT
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
  { NULL, PREC_NONE }, // TOKEN_EMBEDDED
  { NULL, PREC_NONE }, // TOKEN_NAME
  { NULL, PREC_NONE }, // TOKEN_NUMBER
  { NULL, PREC_NONE }, // TOKEN_STRING
  { NULL, PREC_NONE }, // TOKEN_LINE
  { NULL, PREC_NONE }, // TOKEN_WHITESPACE
  { NULL, PREC_NONE }, // TOKEN_INDENT
  { NULL, PREC_NONE }, // TOKEN_OUTDENT
  { NULL, PREC_NONE }, // TOKEN_ERROR
  { NULL, PREC_NONE } // TOKEN_EOF
};

Block* compile(Buffer* source, Token* tokens)
{
  Compiler compiler;
  compiler.source = source;
  compiler.current = tokens;
  compiler.hasError = 0;

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
static void block(Compiler* compiler)
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

static void statementLike(Compiler* compiler)
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

static void expression(Compiler* compiler)
{
  compilePrecedence(compiler, PREC_LOWEST);
}

void compilePrecedence(Compiler* compiler, int precedence)
{
  Token* token = advance(compiler);
  CompileFn prefix = prefixCompilers[token->type];

  if (prefix == NULL)
  {
    // TODO(bob): Handle error better.
    error(compiler, "No prefix parser.");
    exit(1);
  }

  prefix(compiler, token);

  while (precedence <= infixCompilers[compiler->current->type].precedence)
  {
    token = advance(compiler);
    CompileFn infix = infixCompilers[token->type].fn;
    infix(compiler, token);
  }
}

static void prefixLiteral(Compiler* compiler, Token* token)
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

static void infixCall(Compiler* compiler, Token* token)
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

static void infixBinaryOp(Compiler* compiler, Token* token)
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

static TokenType peek(Compiler* compiler)
{
  return compiler->current->type;
}

static Token* match(Compiler* compiler, TokenType expected)
{
  if (peek(compiler) != expected) return NULL;

  return advance(compiler);
}

static Token* consume(Compiler* compiler, TokenType expected)
{
  Token* token = advance(compiler);
  if (token->type != expected)
  {
    // TODO(bob): Better error.
    error(compiler, "Expected %d, got %d.\n", expected, token->type);
  }

  return token;
}

static Token* advance(Compiler* compiler)
{
  // TODO(bob): Check for EOF.
  Token* token = compiler->current;
  compiler->current = compiler->current->next;
  return unlinkToken(token);
}

static void error(Compiler* compiler, const char* format, ...)
{
  compiler->hasError = 1;
  printf("Compile error on '");
  printToken(compiler->source, compiler->current);

  printf("': ");

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}
