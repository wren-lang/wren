#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitives.h"

#define PRIMITIVE(cls, name, prim) \
    { \
      int symbol = ensureSymbol(&vm->symbols, name, strlen(name)); \
      cls->methods[symbol].type = METHOD_PRIMITIVE; \
      cls->methods[symbol].primitive = primitive_##prim; \
    }

#define DEF_PRIMITIVE(prim) \
    static Value primitive_##prim(VM* vm, Fiber* fiber, Value* args, \
                                  int numArgs)

#define GLOBAL(cls, name) \
    { \
      ObjInstance* obj = makeInstance(cls); \
      int symbol = addSymbol(&vm->globalSymbols, name, strlen(name)); \
      vm->globals[symbol] = (Value)obj; \
    }

DEF_PRIMITIVE(block_call)
{
  // The call instruction leading to this primitive has one argument. So when
  // we push the block onto the callstack, we again use one argument. That
  // ensures that the result of evaluating the block goes into the slot that
  // the caller of *this* primitive is expecting.
  callBlock(fiber, AS_BLOCK(args[0]), 1);
  return NULL;
}

DEF_PRIMITIVE(bool_toString)
{
  // TODO(bob): Intern these strings or something.
  if (AS_BOOL(args[0]))
  {
    char* result = malloc(5);
    strcpy(result, "true");
    return (Value)makeString(result);
  }
  else
  {
    char* result = malloc(6);
    strcpy(result, "false");
    return (Value)makeString(result);
  }
}

DEF_PRIMITIVE(num_abs)
{
  double value = AS_NUM(args[0]);
  if (value < 0) value = -value;

  return (Value)makeNum(value);
}

DEF_PRIMITIVE(num_toString)
{
  // TODO(bob): What size should this be?
  char temp[100];
  sprintf(temp, "%g", AS_NUM(args[0]));

  size_t size = strlen(temp) + 1;
  char* result = malloc(size);
  strncpy(result, temp, size);
  return (Value)makeString(result);
}

DEF_PRIMITIVE(num_minus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(AS_NUM(args[0]) - AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_plus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  // TODO(bob): Handle coercion to string if RHS is a string.
  return (Value)makeNum(AS_NUM(args[0]) + AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_multiply)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(AS_NUM(args[0]) * AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_divide)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(AS_NUM(args[0]) / AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_lt)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return makeBool(AS_NUM(args[0]) < AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_gt)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return makeBool(AS_NUM(args[0]) > AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_lte)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return makeBool(AS_NUM(args[0]) <= AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_gte)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return makeBool(AS_NUM(args[0]) >= AS_NUM(args[1]));
}

DEF_PRIMITIVE(string_contains)
{
  const char* string = AS_STRING(args[0]);
  // TODO(bob): Check type of arg first!
  const char* search = AS_STRING(args[1]);

  // Corner case, the empty string contains the empty string.
  if (strlen(string) == 0 && strlen(search) == 0) return (Value)makeNum(1);

  // TODO(bob): Return bool.
  return (Value)makeNum(strstr(string, search) != NULL);
}

DEF_PRIMITIVE(string_count)
{
  double count = strlen(AS_STRING(args[0]));
  return (Value)makeNum(count);
}

DEF_PRIMITIVE(string_toString)
{
  return args[0];
}

DEF_PRIMITIVE(string_plus)
{
  if (args[1]->type != OBJ_STRING) return vm->unsupported;
  // TODO(bob): Handle coercion to string of RHS.

  const char* left = AS_STRING(args[0]);
  const char* right = AS_STRING(args[1]);

  size_t leftLength = strlen(left);
  size_t rightLength = strlen(right);

  char* result = malloc(leftLength + rightLength);
  strcpy(result, left);
  strcpy(result + leftLength, right);
  
  return (Value)makeString(result);
}

DEF_PRIMITIVE(io_write)
{
  printValue(args[1]);
  printf("\n");
  return args[1];
}

void registerPrimitives(VM* vm)
{
  vm->blockClass = makeClass();
  PRIMITIVE(vm->blockClass, "call", block_call);

  vm->boolClass = makeClass();
  PRIMITIVE(vm->boolClass, "toString", bool_toString);

  vm->classClass = makeClass();

  vm->numClass = makeClass();
  PRIMITIVE(vm->numClass, "abs", num_abs);
  PRIMITIVE(vm->numClass, "toString", num_toString)
  PRIMITIVE(vm->numClass, "- ", num_minus);
  PRIMITIVE(vm->numClass, "+ ", num_plus);
  PRIMITIVE(vm->numClass, "* ", num_multiply);
  PRIMITIVE(vm->numClass, "/ ", num_divide);
  PRIMITIVE(vm->numClass, "< ", num_lt);
  PRIMITIVE(vm->numClass, "> ", num_gt);
  PRIMITIVE(vm->numClass, "<= ", num_lte);
  PRIMITIVE(vm->numClass, ">= ", num_gte);

  vm->stringClass = makeClass();
  PRIMITIVE(vm->stringClass, "contains ", string_contains);
  PRIMITIVE(vm->stringClass, "count", string_count);
  PRIMITIVE(vm->stringClass, "toString", string_toString)
  PRIMITIVE(vm->stringClass, "+ ", string_plus);

  ObjClass* ioClass = makeClass();
  PRIMITIVE(ioClass, "write ", io_write);
  GLOBAL(ioClass, "io");

  ObjClass* unsupportedClass = makeClass();

  // TODO(bob): Make this a distinct object type.
  vm->unsupported = (Value)makeInstance(unsupportedClass);
}