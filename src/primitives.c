#include <stdlib.h>
#include <string.h>

#include "primitives.h"

#define PRIMITIVE(cls, name, prim) \
    { \
      int symbol = ensureSymbol(&vm->symbols, name, strlen(name)); \
      vm->cls##Class->methods[symbol].type = METHOD_PRIMITIVE; \
      vm->cls##Class->methods[symbol].primitive = primitive_##cls##_##prim; \
    }

#define DEF_PRIMITIVE(cls, prim) \
    static Value primitive_##cls##_##prim(Value* args, int numArgs)

DEF_PRIMITIVE(num, abs)
{
  double value = ((ObjNum*)args[0])->value;
  if (value < 0) value = -value;

  return (Value)makeNum(value);
}

DEF_PRIMITIVE(string, contains)
{
  const char* string = ((ObjString*)args[0])->value;
  // TODO(bob): Check type of arg first!
  const char* search = ((ObjString*)args[1])->value;

  // TODO(bob): Return bool.
  return (Value)makeNum(strstr(string, search) != NULL);
}

DEF_PRIMITIVE(string, count)
{
  double count = strlen(((ObjString*)args[0])->value);

  return (Value)makeNum(count);
}

void registerPrimitives(VM* vm)
{
  PRIMITIVE(num, "abs", abs);
  PRIMITIVE(string, "contains ", contains);
  PRIMITIVE(string, "count", count);
}