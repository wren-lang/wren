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
    static Value primitive_##prim(VM* vm, Value* args, int numArgs)

#define GLOBAL(cls, name) \
    { \
      ObjInstance* obj = makeInstance(cls); \
      int symbol = addSymbol(&vm->globalSymbols, name, strlen(name)); \
      vm->globals[symbol] = (Value)obj; \
    }

DEF_PRIMITIVE(num_abs)
{
  double value = ((ObjNum*)args[0])->value;
  if (value < 0) value = -value;

  return (Value)makeNum(value);
}

DEF_PRIMITIVE(num_minus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(((ObjNum*)args[0])->value - ((ObjNum*)args[1])->value);
}

DEF_PRIMITIVE(num_plus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  // TODO(bob): Handle coercion to string if RHS is a string.
  return (Value)makeNum(((ObjNum*)args[0])->value + ((ObjNum*)args[1])->value);
}

DEF_PRIMITIVE(num_multiply)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(((ObjNum*)args[0])->value * ((ObjNum*)args[1])->value);
}

DEF_PRIMITIVE(num_divide)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)makeNum(((ObjNum*)args[0])->value / ((ObjNum*)args[1])->value);
}

DEF_PRIMITIVE(string_contains)
{
  const char* string = ((ObjString*)args[0])->value;
  // TODO(bob): Check type of arg first!
  const char* search = ((ObjString*)args[1])->value;

  // Corner case, the empty string contains the empty string.
  if (strlen(string) == 0 && strlen(search) == 0) return (Value)makeNum(1);

  // TODO(bob): Return bool.
  return (Value)makeNum(strstr(string, search) != NULL);
}

DEF_PRIMITIVE(string_count)
{
  double count = strlen(((ObjString*)args[0])->value);

  return (Value)makeNum(count);
}

DEF_PRIMITIVE(string_plus)
{
  if (args[1]->type != OBJ_STRING) return vm->unsupported;
  // TODO(bob): Handle coercion to string of RHS.

  ObjString* left = (ObjString*)args[0];
  ObjString* right = (ObjString*)args[1];

  size_t leftLength = strlen(left->value);
  size_t rightLength = strlen(right->value);

  char* result = malloc(leftLength + rightLength);
  strcpy(result, left->value);
  strcpy(result + leftLength, right->value);
  
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
  PRIMITIVE(vm->numClass, "abs", num_abs);
  PRIMITIVE(vm->numClass, "- ", num_minus);
  PRIMITIVE(vm->numClass, "+ ", num_plus);
  PRIMITIVE(vm->numClass, "* ", num_multiply);
  PRIMITIVE(vm->numClass, "/ ", num_divide);

  PRIMITIVE(vm->stringClass, "contains ", string_contains);
  PRIMITIVE(vm->stringClass, "count", string_count);
  PRIMITIVE(vm->stringClass, "+ ", string_plus);

  ObjClass* ioClass = makeClass();
  PRIMITIVE(ioClass, "write ", io_write);
  GLOBAL(ioClass, "io");

  ObjClass* unsupportedClass = makeClass();

  // TODO(bob): Make this a distinct object type.
  vm->unsupported = (Value)makeInstance(unsupportedClass);
}