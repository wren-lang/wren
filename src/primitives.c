#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "primitives.h"

#define PRIMITIVE(cls, name, prim) \
    { \
      int symbol = ensureSymbol(&vm->methods, name, strlen(name)); \
      cls->methods[symbol].type = METHOD_PRIMITIVE; \
      cls->methods[symbol].primitive = primitive_##prim; \
    }

#define DEF_PRIMITIVE(prim) \
    static Value primitive_##prim(VM* vm, Fiber* fiber, Value* args)

DEF_PRIMITIVE(bool_not)
{
  return newBool(vm, !AS_BOOL(args[0]));
}

DEF_PRIMITIVE(bool_eqeq)
{
  if (args[1]->type != OBJ_FALSE && args[1]->type != OBJ_TRUE)
  {
    return newBool(vm, 0);
  }

  return newBool(vm, AS_BOOL(args[0]) == AS_BOOL(args[1]));
}

DEF_PRIMITIVE(bool_bangeq)
{
  if (args[1]->type != OBJ_FALSE && args[1]->type != OBJ_TRUE)
  {
    return newBool(vm, 1);
  }

  return newBool(vm, AS_BOOL(args[0]) != AS_BOOL(args[1]));
}

DEF_PRIMITIVE(bool_toString)
{
  // TODO(bob): Intern these strings or something.
  if (AS_BOOL(args[0]))
  {
    return (Value)newString(vm, "true", 4);
  }
  else
  {
    return (Value)newString(vm, "false", 5);
  }
}

DEF_PRIMITIVE(fn_call)
{
  // The call instruction leading to this primitive has one argument. So when
  // we push the block onto the callstack, we again use one argument. That
  // ensures that the result of evaluating the block goes into the slot that
  // the caller of *this* primitive is expecting.
  callFunction(fiber, AS_FN(args[0]), 1);
  return NULL;
}

DEF_PRIMITIVE(fn_eqeq)
{
  if (args[1]->type != OBJ_FN) return newBool(vm, 0);
  return newBool(vm, AS_FN(args[0]) == AS_FN(args[1]));
}

DEF_PRIMITIVE(fn_bangeq)
{
  if (args[1]->type != OBJ_FN) return newBool(vm, 1);
  return newBool(vm, AS_FN(args[0]) != AS_FN(args[1]));
}

DEF_PRIMITIVE(num_abs)
{
  return (Value)newNum(vm, fabs(AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_toString)
{
  // TODO(bob): What size should this be?
  char temp[100];
  sprintf(temp, "%g", AS_NUM(args[0]));
  return (Value)newString(vm, temp, strlen(temp));
}

DEF_PRIMITIVE(num_negate)
{
  return (Value)newNum(vm, -AS_NUM(args[0]));
}

DEF_PRIMITIVE(num_minus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)newNum(vm, AS_NUM(args[0]) - AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_plus)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  // TODO(bob): Handle coercion to string if RHS is a string.
  return (Value)newNum(vm, AS_NUM(args[0]) + AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_multiply)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)newNum(vm, AS_NUM(args[0]) * AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_divide)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)newNum(vm, AS_NUM(args[0]) / AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_mod)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return (Value)newNum(vm, fmod(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_lt)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return newBool(vm, AS_NUM(args[0]) < AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_gt)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return newBool(vm, AS_NUM(args[0]) > AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_lte)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return newBool(vm, AS_NUM(args[0]) <= AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_gte)
{
  if (args[1]->type != OBJ_NUM) return vm->unsupported;
  return newBool(vm, AS_NUM(args[0]) >= AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_eqeq)
{
  if (args[1]->type != OBJ_NUM) return newBool(vm, 0);
  return newBool(vm, AS_NUM(args[0]) == AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_bangeq)
{
  if (args[1]->type != OBJ_NUM) return newBool(vm, 1);
  return newBool(vm, AS_NUM(args[0]) != AS_NUM(args[1]));
}

DEF_PRIMITIVE(string_contains)
{
  const char* string = AS_STRING(args[0]);
  // TODO(bob): Check type of arg first!
  const char* search = AS_STRING(args[1]);

  // Corner case, the empty string contains the empty string.
  if (strlen(string) == 0 && strlen(search) == 0) return (Value)newNum(vm, 1);

  // TODO(bob): Return bool.
  return (Value)newNum(vm, strstr(string, search) != NULL);
}

DEF_PRIMITIVE(string_count)
{
  double count = strlen(AS_STRING(args[0]));
  return (Value)newNum(vm, count);
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

  ObjString* string = newString(vm, NULL, leftLength + rightLength);
  strcpy(string->value, left);
  strcpy(string->value + leftLength, right);
  string->value[leftLength + rightLength] = '\0';

  return (Value)string;
}

DEF_PRIMITIVE(string_eqeq)
{
  if (args[1]->type != OBJ_STRING) return newBool(vm, 0);
  const char* a = AS_STRING(args[0]);
  const char* b = AS_STRING(args[1]);
  return newBool(vm, strcmp(a, b) == 0);
}

DEF_PRIMITIVE(string_bangeq)
{
  if (args[1]->type != OBJ_STRING) return newBool(vm, 1);
  const char* a = AS_STRING(args[0]);
  const char* b = AS_STRING(args[1]);
  return newBool(vm, strcmp(a, b) != 0);
}

DEF_PRIMITIVE(io_write)
{
  printValue(args[1]);
  printf("\n");
  return args[1];
}

static const char* CORE_LIB =
"class Object {}\n"
"class Bool {}\n"
"class Class {}\n"
"class Function {}\n"
"class Num {}\n"
"class Null {}\n"
"class String {}\n"
"class IO {}\n"
"var io = IO.new\n";

void loadCore(VM* vm)
{
  ObjFn* core = compile(vm, CORE_LIB);
  interpret(vm, core);

  vm->boolClass = AS_CLASS(findGlobal(vm, "Bool"));
  PRIMITIVE(vm->boolClass, "toString", bool_toString);
  PRIMITIVE(vm->boolClass, "!", bool_not);
  PRIMITIVE(vm->boolClass, "== ", bool_eqeq);
  PRIMITIVE(vm->boolClass, "!= ", bool_bangeq);

  vm->classClass = AS_CLASS(findGlobal(vm, "Class"));

  vm->fnClass = AS_CLASS(findGlobal(vm, "Function"));
  PRIMITIVE(vm->fnClass, "call", fn_call);
  PRIMITIVE(vm->fnClass, "== ", fn_eqeq);
  PRIMITIVE(vm->fnClass, "!= ", fn_bangeq);

  vm->nullClass = AS_CLASS(findGlobal(vm, "Null"));

  vm->numClass = AS_CLASS(findGlobal(vm, "Num"));
  PRIMITIVE(vm->numClass, "abs", num_abs);
  PRIMITIVE(vm->numClass, "toString", num_toString)
  PRIMITIVE(vm->numClass, "-", num_negate);
  PRIMITIVE(vm->numClass, "- ", num_minus);
  PRIMITIVE(vm->numClass, "+ ", num_plus);
  PRIMITIVE(vm->numClass, "* ", num_multiply);
  PRIMITIVE(vm->numClass, "/ ", num_divide);
  PRIMITIVE(vm->numClass, "% ", num_mod);
  PRIMITIVE(vm->numClass, "< ", num_lt);
  PRIMITIVE(vm->numClass, "> ", num_gt);
  PRIMITIVE(vm->numClass, "<= ", num_lte);
  PRIMITIVE(vm->numClass, ">= ", num_gte);
  PRIMITIVE(vm->numClass, "== ", num_eqeq);
  PRIMITIVE(vm->numClass, "!= ", num_bangeq);

  vm->objectClass = AS_CLASS(findGlobal(vm, "Object"));

  vm->stringClass = AS_CLASS(findGlobal(vm, "String"));
  PRIMITIVE(vm->stringClass, "contains ", string_contains);
  PRIMITIVE(vm->stringClass, "count", string_count);
  PRIMITIVE(vm->stringClass, "toString", string_toString)
  PRIMITIVE(vm->stringClass, "+ ", string_plus);
  PRIMITIVE(vm->stringClass, "== ", string_eqeq);
  PRIMITIVE(vm->stringClass, "!= ", string_bangeq);

  ObjClass* ioClass = AS_CLASS(findGlobal(vm, "IO"));
  PRIMITIVE(ioClass, "write ", io_write);

  ObjClass* unsupportedClass = newClass(vm, vm->objectClass);

  // TODO(bob): Make this a distinct object type.
  vm->unsupported = (Value)newInstance(vm, unsupportedClass);
}
