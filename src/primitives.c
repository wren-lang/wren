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
    static Value primitive_##prim(Value* args, int numArgs)

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

DEF_PRIMITIVE(string_contains)
{
  const char* string = ((ObjString*)args[0])->value;
  // TODO(bob): Check type of arg first!
  const char* search = ((ObjString*)args[1])->value;

  // TODO(bob): Return bool.
  return (Value)makeNum(strstr(string, search) != NULL);
}

DEF_PRIMITIVE(string_count)
{
  double count = strlen(((ObjString*)args[0])->value);

  return (Value)makeNum(count);
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
  PRIMITIVE(vm->stringClass, "contains ", string_contains);
  PRIMITIVE(vm->stringClass, "count", string_count);

  ObjClass* ioClass = makeClass();
  PRIMITIVE(ioClass, "write ", io_write);
  GLOBAL(ioClass, "io");
}