#include <stdio.h>
#include <stdarg.h>
#include "parser.h"

typedef struct
{
  Buffer* source;
  Token* current;

  // Non-zero if a parse error has occurred.
  int hasError;
} Parser;

typedef Node* (*PrefixParserFn)(Parser*, Token*);
typedef Node* (*InfixParserFn)(Parser*, Node*, Token*);

typedef struct
{
  InfixParserFn fn;
  int precedence;
} InfixParser;

static Node* block(Parser* parser);
static Node* statementLike(Parser* parser);
static Node* expression(Parser* parser);
static Node* parsePrecedence(Parser* parser, int precedence);
static Node* prefixLiteral(Parser* parser, Token* token);
static Node* infixCall(Parser* parser, Node* left, Token* token);
static Node* infixBinaryOp(Parser* parser, Node* left, Token* token);
static TokenType peek(Parser* parser);
static Token* match(Parser* parser, TokenType expected);
static Token* consume(Parser* parser, TokenType expected);
static Token* advance(Parser* parser);
static void error(Parser* parser, const char* format, ...);

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

PrefixParserFn prefixParsers[] = {
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
InfixParser infixParsers[] = {
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

Node* parse(Buffer* source, Token* tokens)
{
  Parser parser;
  parser.source = source;
  parser.current = tokens;
  parser.hasError = 0;

  NodeSequence* sequence = (NodeSequence*)malloc(sizeof(NodeSequence));
  sequence->node.type = NODE_SEQUENCE;
  sequence->nodes = NULL;

  // TODO(bob): Copied from block(). Unify.
  NodeList** nodes = &sequence->nodes;
  do
  {
    Node* node = statementLike(&parser);
    *nodes = (NodeList*)malloc(sizeof(NodeList));
    (*nodes)->node = node;
    (*nodes)->next = NULL;
    nodes = &(*nodes)->next;

  } while (!match(&parser, TOKEN_EOF));

  return parser.hasError ? NULL : (Node*)sequence;
}

static Node* block(Parser* parser)
{
  consume(parser, TOKEN_INDENT);

  NodeSequence* sequence = (NodeSequence*)malloc(sizeof(NodeSequence));
  sequence->node.type = NODE_SEQUENCE;
  sequence->nodes = NULL;

  NodeList** nodes = &sequence->nodes;
  do
  {
    Node* node = statementLike(parser);
    *nodes = (NodeList*)malloc(sizeof(NodeList));
    (*nodes)->node = node;
    (*nodes)->next = NULL;
    nodes = &(*nodes)->next;

  } while (!match(parser, TOKEN_OUTDENT));

  return (Node*)sequence;
}

static Node* statementLike(Parser* parser)
{
  if (match(parser, TOKEN_IF))
  {
    Node* condition = expression(parser);
    consume(parser, TOKEN_COLON);
    Node* thenArm = block(parser);
    Node* elseArm = NULL;
    if (match(parser, TOKEN_ELSE))
    {
      consume(parser, TOKEN_COLON);
      elseArm = block(parser);
    }

    NodeIf* expr = (NodeIf*)malloc(sizeof(NodeIf));
    expr->node.type = NODE_IF;
    expr->condition = condition;
    expr->thenArm = thenArm;
    expr->elseArm = elseArm;
    return (Node*)expr;
  }

  if (match(parser, TOKEN_VAR))
  {
    Token* name = consume(parser, TOKEN_NAME);
    Node* initializer = NULL;
    if (match(parser, TOKEN_EQ))
    {
      initializer = expression(parser);
    }
    if (peek(parser) != TOKEN_OUTDENT) consume(parser, TOKEN_LINE);

    NodeVar* node = (NodeVar*)malloc(sizeof(NodeVar));
    node->node.type = NODE_VAR;
    node->name = name;
    node->initializer = initializer;
    return (Node*)node;
  }

  // Statement expression.
  Node* node = expression(parser);
  if (peek(parser) != TOKEN_OUTDENT) consume(parser, TOKEN_LINE);
  
  return node;
}

static Node* expression(Parser* parser)
{
  return parsePrecedence(parser, PREC_LOWEST);
}

Node* parsePrecedence(Parser* parser, int precedence)
{
  Token* token = advance(parser);
  PrefixParserFn prefix = prefixParsers[token->type];

  if (prefix == NULL)
  {
    // TODO(bob): Handle error better.
    error(parser, "No prefix parser.");
    exit(1);
  }

  Node* left = prefix(parser, token);

  while (precedence <= infixParsers[parser->current->type].precedence)
  {
    token = advance(parser);
    InfixParserFn infix = infixParsers[token->type].fn;
    left = infix(parser, left, token);
  }

  return left;
}

static Node* prefixLiteral(Parser* parser, Token* token)
{
  NodeLiteral* node = (NodeLiteral*)malloc(sizeof(NodeLiteral));
  node->node.type = NODE_LITERAL;
  node->token = token;
  return (Node*)node;
}

static Node* infixCall(Parser* parser, Node* left, Token* token)
{
  NodeList* args = NULL;
  if (match(parser, TOKEN_RIGHT_PAREN) == NULL)
  {
    NodeList** arg = &args;
    do
    {
      *arg = (NodeList*)malloc(sizeof(NodeList));
      (*arg)->node = expression(parser);
      (*arg)->next = NULL;
      arg = &(*arg)->next;
    }
    while (match(parser, TOKEN_COMMA) != NULL);

    consume(parser, TOKEN_RIGHT_PAREN);
  }

  NodeCall* node = (NodeCall*)malloc(sizeof(NodeCall));
  node->node.type = NODE_CALL;
  node->fn = left;
  node->args = args;

  return (Node*)node;
}

static Node* infixBinaryOp(Parser* parser, Node* left, Token* token)
{
  // TODO(bob): Support right-associative infix. Needs to do precedence
  // - 1 here to be right-assoc.
  Node* right = parsePrecedence(parser,
                                infixParsers[token->type].precedence);

  NodeBinaryOp* node = (NodeBinaryOp*)malloc(sizeof(NodeBinaryOp));
  node->node.type = NODE_BINARY_OP;
  node->left = left;
  node->op = token;
  node->right = right;

  return (Node*)node;
}

static TokenType peek(Parser* parser)
{
  return parser->current->type;
}

static Token* match(Parser* parser, TokenType expected)
{
  if (peek(parser) != expected) return NULL;

  return advance(parser);
}

static Token* consume(Parser* parser, TokenType expected)
{
  Token* token = advance(parser);
  if (token->type != expected)
  {
    // TODO(bob): Better error.
    error(parser, "Expected %d, got %d.\n", expected, token->type);
  }

  return token;
}

static Token* advance(Parser* parser)
{
  // TODO(bob): Check for EOF.
  Token* token = parser->current;
  parser->current = parser->current->next;
  return unlinkToken(token);
}

static void error(Parser* parser, const char* format, ...)
{
  parser->hasError = 1;
  printf("Parse error on '");
  printToken(parser->source, parser->current);

  printf("': ");

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
}
