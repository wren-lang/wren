#ifndef wren_parser_h
#define wren_parser_h

#include "lexer.h"

// AST nodes.

typedef enum
{
  NODE_LITERAL,
  NODE_SEQUENCE,
  NODE_CALL,
  NODE_BINARY_OP,
  NODE_IF,
  NODE_VAR,

  NODE_MAX
} NodeType;

typedef struct
{
  NodeType type;
} Node;

typedef struct NodeList_s
{
  Node* node;
  struct NodeList_s* next;
} NodeList;

// Numbers, strings, variable names.
typedef struct
{
  Node node;
  Token* token;
} NodeLiteral;

typedef struct
{
  Node node;
  NodeList* nodes;
} NodeSequence;

typedef struct
{
  Node node;
  Node* fn;
  NodeList* args;
} NodeCall;

typedef struct
{
  Node node;
  Token* op;
  Node* left;
  Node* right;
} NodeBinaryOp;

typedef struct
{
  Node node;
  Node* condition;
  Node* thenArm;
  Node* elseArm;
} NodeIf;

typedef struct
{
  Node node;
  Token* name;
  Node* initializer;
} NodeVar;

// Parameters.

// TODO(bob): Is this needed?
typedef struct
{
  Token* name;
} Param;

typedef struct ParamList_s
{
  Param* param;
  struct ParamList_s* next;
} ParamList;

Node* parse(Buffer* source, Token* tokens);

#endif
