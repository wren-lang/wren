#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "wren_core.h"
#include "wren_value.h"

// Binds a native method named [name] (in Wren) implemented using C function
// [fn] to `ObjClass` [cls].
#define NATIVE(cls, name, fn) \
    { \
      int symbol = ensureSymbol(vm, &vm->methods, name, strlen(name)); \
      cls->methods[symbol].type = METHOD_PRIMITIVE; \
      cls->methods[symbol].primitive = native_##fn; \
    }

// Defines a native method whose C function name is [native]. This abstracts
// the actual type signature of a native function and makes it clear which C
// functions are intended to be invoked as natives.
#define DEF_NATIVE(native) \
    static PrimitiveResult native_##native(WrenVM* vm, ObjFiber* fiber, Value* args)

#define RETURN_VAL(value)   do { args[0] = value; return PRIM_VALUE; } while (0)

#define RETURN_BOOL(value)  RETURN_VAL(BOOL_VAL(value))
#define RETURN_FALSE        RETURN_VAL(FALSE_VAL)
#define RETURN_NULL         RETURN_VAL(NULL_VAL)
#define RETURN_NUM(value)   RETURN_VAL(NUM_VAL(value))
#define RETURN_TRUE         RETURN_VAL(TRUE_VAL)

#define RETURN_ERROR(msg) \
    do { \
      args[0] = wrenNewString(vm, msg, strlen(msg)); \
      return PRIM_ERROR; \
    } while (0);

// This string literal is generated automatically from corelib.wren using
// make_corelib. Do not edit here.
const char* coreLibSource =
"class IO {\n"
"  static write(obj) {\n"
"    IO.write__native__(obj.toString)\n"
"    return obj\n"
"  }\n"
"}\n"
"\n"
"class List {\n"
"  toString {\n"
"    var result = \"[\"\n"
"    var i = 0\n"
"    // TODO: Use for loop.\n"
"    while (i < this.count) {\n"
"      if (i > 0) result = result + \", \"\n"
"      result = result + this[i].toString\n"
"      i = i + 1\n"
"    }\n"
"    result = result + \"]\"\n"
"    return result\n"
"  }\n"
"}\n"
"\n"
"class Range {\n"
"  new(min, max) {\n"
"    _min = min\n"
"    _max = max\n"
"  }\n"
"\n"
"  min { return _min }\n"
"  max { return _max }\n"
"\n"
"  iterate(previous) {\n"
"    if (previous == null) return _min\n"
"    if (previous == _max) return false\n"
"    return previous + 1\n"
"  }\n"
"\n"
"  iteratorValue(iterator) {\n"
"    return iterator\n"
"  }\n"
"}\n"
"\n"
"class Num {\n"
"  .. other { return new Range(this, other) }\n"
"  ... other { return new Range(this, other - 1) }\n"
"}\n";

DEF_NATIVE(bool_not)
{
  RETURN_BOOL(!AS_BOOL(args[0]));
}

DEF_NATIVE(bool_toString)
{
  if (AS_BOOL(args[0]))
  {
    RETURN_VAL(wrenNewString(vm, "true", 4));
  }
  else
  {
    RETURN_VAL(wrenNewString(vm, "false", 5));
  }
}

DEF_NATIVE(fn_call) { return PRIM_CALL; }

// Validates that [index] is an integer within `[0, count)`. Also allows
// negative indices which map backwards from the end. Returns the valid positive
// index value, or -1 if the index wasn't valid (not a number, not an int, out
// of bounds).
static int validateIndex(Value index, int count)
{
  if (!IS_NUM(index)) return -1;

  double indexNum = AS_NUM(index);
  int intIndex = (int)indexNum;
  // Make sure the index is an integer.
  if (indexNum != intIndex) return -1;

  // Negative indices count from the end.
  if (indexNum < 0) indexNum = count + indexNum;

  // Check bounds.
  if (indexNum < 0 || indexNum >= count) return -1;

  return indexNum;
}

DEF_NATIVE(list_add)
{
  ObjList* list = AS_LIST(args[0]);
  wrenListAdd(vm, list, args[1]);
  RETURN_VAL(args[1]);
}

DEF_NATIVE(list_clear)
{
  ObjList* list = AS_LIST(args[0]);
  wrenReallocate(vm, list->elements, 0, 0);
  list->elements = NULL;
  list->capacity = 0;
  list->count = 0;
  RETURN_NULL;
}

DEF_NATIVE(list_count)
{
  ObjList* list = AS_LIST(args[0]);
  RETURN_NUM(list->count);
}

DEF_NATIVE(list_insert)
{
  ObjList* list = AS_LIST(args[0]);

  // count + 1 here so you can "insert" at the very end.
  int index = validateIndex(args[2], list->count + 1);
  // TODO: Instead of returning null here, should signal an error explicitly
  // somehow.
  if (index == -1) RETURN_NULL;

  wrenListInsert(vm, list, args[1], index);
  RETURN_VAL(args[1]);
}

DEF_NATIVE(list_iterate)
{
  // If we're starting the iteration, return the first index.
  if (IS_NULL(args[1])) RETURN_NUM(0);

  ObjList* list = AS_LIST(args[0]);
  double index = AS_NUM(args[1]);
  // TODO: Handle arg not a number or not an integer.

  // Stop if we're out of elements.
  if (index >= list->count - 1) RETURN_FALSE;

  // Otherwise, move to the next index.
  RETURN_NUM(index + 1);
}

DEF_NATIVE(list_iteratorValue)
{
  ObjList* list = AS_LIST(args[0]);
  double index = AS_NUM(args[1]);
  // TODO: Handle index out of bounds or not integer.
  RETURN_VAL(list->elements[(int)index]);
}

DEF_NATIVE(list_removeAt)
{
  ObjList* list = AS_LIST(args[0]);
  int index = validateIndex(args[1], list->count);
  // TODO: Instead of returning null here, should signal an error explicitly
  // somehow.
  if (index == -1) RETURN_NULL;

  RETURN_VAL(wrenListRemoveAt(vm, list, index));
}

DEF_NATIVE(list_subscript)
{
  ObjList* list = AS_LIST(args[0]);

  int index = validateIndex(args[1], list->count);
  // TODO: Instead of returning null here, should signal an error explicitly
  // somehow.
  if (index == -1) RETURN_NULL;

  RETURN_VAL(list->elements[index]);
}

DEF_NATIVE(list_subscriptSetter)
{
  ObjList* list = AS_LIST(args[0]);

  int index = validateIndex(args[1], list->count);
  // TODO: Instead of returning null here, should signal an error explicitly
  // somehow.
  if (index == -1) RETURN_NULL;

  list->elements[index] = args[2];
  RETURN_VAL(args[2]);
}

DEF_NATIVE(null_toString)
{
  RETURN_VAL(wrenNewString(vm, "null", 4));
}

DEF_NATIVE(num_abs)
{
  RETURN_NUM(fabs(AS_NUM(args[0])));
}

DEF_NATIVE(num_toString)
{
  // According to Lua implementation, this should be enough for longest number
  // formatted using %.14g.
  char buffer[21];
  sprintf(buffer, "%.14g", AS_NUM(args[0]));
  RETURN_VAL(wrenNewString(vm, buffer, strlen(buffer)));
}

DEF_NATIVE(num_negate)
{
  RETURN_NUM(-AS_NUM(args[0]));
}

// Validates that the second argument in [args] is a Num. Returns true if it is.
// If not, sets a type error and returns false.
static bool checkNumType(WrenVM* vm, Value* args)
{
  if (IS_NUM(args[1])) return true;

  const char* msg = "TypeError: Right operand must be Num.";
  args[0] = wrenNewString(vm, msg, strlen(msg));
  return false;
}

DEF_NATIVE(num_minus)
{
  if (!checkNumType(vm, args)) return PRIM_ERROR;
  RETURN_NUM(AS_NUM(args[0]) - AS_NUM(args[1]));
}

DEF_NATIVE(num_plus)
{
  if (!checkNumType(vm, args)) return PRIM_ERROR;
  // TODO: Handle coercion to string if RHS is a string.
  RETURN_NUM(AS_NUM(args[0]) + AS_NUM(args[1]));
}

DEF_NATIVE(num_multiply)
{
  if (!checkNumType(vm, args)) return PRIM_ERROR;
  RETURN_NUM(AS_NUM(args[0]) * AS_NUM(args[1]));
}

DEF_NATIVE(num_divide)
{
  if (!checkNumType(vm, args)) return PRIM_ERROR;
  RETURN_NUM(AS_NUM(args[0]) / AS_NUM(args[1]));
}

DEF_NATIVE(num_mod)
{
  if (!checkNumType(vm, args)) return PRIM_ERROR;
  if (!IS_NUM(args[1])) RETURN_NULL;
  RETURN_NUM(fmod(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_NATIVE(num_lt)
{
  // TODO: Error on wrong argument type.
  if (!IS_NUM(args[1])) RETURN_NULL;
  RETURN_BOOL(AS_NUM(args[0]) < AS_NUM(args[1]));
}

DEF_NATIVE(num_gt)
{
  // TODO: Error on wrong argument type.
  if (!IS_NUM(args[1])) RETURN_NULL;
  RETURN_BOOL(AS_NUM(args[0]) > AS_NUM(args[1]));
}

DEF_NATIVE(num_lte)
{
  // TODO: Error on wrong argument type.
  if (!IS_NUM(args[1])) RETURN_NULL;
  RETURN_BOOL(AS_NUM(args[0]) <= AS_NUM(args[1]));
}

DEF_NATIVE(num_gte)
{
  // TODO: Error on wrong argument type.
  if (!IS_NUM(args[1])) RETURN_NULL;
  RETURN_BOOL(AS_NUM(args[0]) >= AS_NUM(args[1]));
}

DEF_NATIVE(num_eqeq)
{
  if (!IS_NUM(args[1])) RETURN_FALSE;
  RETURN_BOOL(AS_NUM(args[0]) == AS_NUM(args[1]));
}

DEF_NATIVE(num_bangeq)
{
  if (!IS_NUM(args[1])) RETURN_TRUE;
  RETURN_BOOL(AS_NUM(args[0]) != AS_NUM(args[1]));
}

DEF_NATIVE(num_bitwiseNot)
{
  // Bitwise operators always work on 32-bit unsigned ints.
  uint32_t value = (uint32_t)AS_NUM(args[0]);
  RETURN_NUM(~value);
}

DEF_NATIVE(object_eqeq)
{
  RETURN_BOOL(wrenValuesEqual(args[0], args[1]));
}

DEF_NATIVE(object_bangeq)
{
  RETURN_BOOL(!wrenValuesEqual(args[0], args[1]));
}

DEF_NATIVE(object_new)
{
  // This is the default argument-less constructor that all objects inherit.
  // It just returns "this".
  RETURN_VAL(args[0]);
}

DEF_NATIVE(object_toString)
{
  RETURN_VAL(wrenNewString(vm, "<object>", 8));
}

DEF_NATIVE(object_type)
{
  RETURN_VAL(OBJ_VAL(wrenGetClass(vm, args[0])));
}

DEF_NATIVE(string_contains)
{
  const char* string = AS_CSTRING(args[0]);
  // TODO: Check type of arg first!
  const char* search = AS_CSTRING(args[1]);

  // Corner case, the empty string contains the empty string.
  if (strlen(string) == 0 && strlen(search) == 0) RETURN_TRUE;

  RETURN_BOOL(strstr(string, search) != NULL);
}

DEF_NATIVE(string_count)
{
  double count = strlen(AS_CSTRING(args[0]));
  RETURN_NUM(count);
}

DEF_NATIVE(string_toString)
{
  RETURN_VAL(args[0]);
}

DEF_NATIVE(string_plus)
{
  if (!IS_STRING(args[1])) RETURN_NULL;
  // TODO: Handle coercion to string of RHS.

  const char* left = AS_CSTRING(args[0]);
  const char* right = AS_CSTRING(args[1]);

  size_t leftLength = strlen(left);
  size_t rightLength = strlen(right);

  Value value = wrenNewString(vm, NULL, leftLength + rightLength);
  ObjString* string = AS_STRING(value);
  strcpy(string->value, left);
  strcpy(string->value + leftLength, right);
  string->value[leftLength + rightLength] = '\0';

  RETURN_VAL(value);
}

DEF_NATIVE(string_eqeq)
{
  if (!IS_STRING(args[1])) RETURN_FALSE;
  const char* a = AS_CSTRING(args[0]);
  const char* b = AS_CSTRING(args[1]);
  RETURN_BOOL(strcmp(a, b) == 0);
}

DEF_NATIVE(string_bangeq)
{
  if (!IS_STRING(args[1])) RETURN_TRUE;
  const char* a = AS_CSTRING(args[0]);
  const char* b = AS_CSTRING(args[1]);
  RETURN_BOOL(strcmp(a, b) != 0);
}

DEF_NATIVE(string_subscript)
{
  // TODO: Instead of returning null here, all of these failure cases should
  // signal an error explicitly somehow.
  if (!IS_NUM(args[1])) RETURN_NULL;

  double indexNum = AS_NUM(args[1]);
  int index = (int)indexNum;
  // Make sure the index is an integer.
  if (indexNum != index) RETURN_NULL;

  ObjString* string = AS_STRING(args[0]);

  // Negative indices count from the end.
  // TODO: Strings should cache their length.
  int length = (int)strlen(string->value);
  if (index < 0) index = length + index;

  // Check bounds.
  if (index < 0 || index >= length) RETURN_NULL;

  // The result is a one-character string.
  // TODO: Handle UTF-8.
  Value value = wrenNewString(vm, NULL, 2);
  ObjString* result = AS_STRING(value);
  result->value[0] = AS_CSTRING(args[0])[index];
  result->value[1] = '\0';
  RETURN_VAL(value);
}

DEF_NATIVE(io_writeString)
{
  wrenPrintValue(args[1]);
  printf("\n");
  RETURN_VAL(args[1]);
}

DEF_NATIVE(os_clock)
{
  double time = (double)clock() / CLOCKS_PER_SEC;
  RETURN_NUM(time);
}

static ObjClass* defineClass(WrenVM* vm, const char* name)
{
  // Add the symbol first since it can trigger a GC.
  int symbol = addSymbol(vm, &vm->globalSymbols, name, strlen(name));

  ObjClass* classObj = wrenNewClass(vm, vm->objectClass, 0);
  vm->globals[symbol] = OBJ_VAL(classObj);
  return classObj;
}

void wrenInitializeCore(WrenVM* vm)
{
  // Define the root Object class. This has to be done a little specially
  // because it has no superclass and an unusual metaclass (Class).
  int objectSymbol = addSymbol(vm, &vm->globalSymbols,
                               "Object", strlen("Object"));
  vm->objectClass = wrenNewSingleClass(vm, 0);
  vm->globals[objectSymbol] = OBJ_VAL(vm->objectClass);

  NATIVE(vm->objectClass, "== ", object_eqeq);
  NATIVE(vm->objectClass, "!= ", object_bangeq);
  NATIVE(vm->objectClass, "new", object_new);
  NATIVE(vm->objectClass, "toString", object_toString);
  NATIVE(vm->objectClass, "type", object_type);

  // Now we can define Class, which is a subclass of Object, but Object's
  // metaclass.
  int classSymbol = addSymbol(vm, &vm->globalSymbols,
                              "Class", strlen("Class"));
  vm->classClass = wrenNewSingleClass(vm, 0);
  vm->globals[classSymbol] = OBJ_VAL(vm->classClass);

  // Now that Object and Class are defined, we can wire them up to each other.
  wrenBindSuperclass(vm->classClass, vm->objectClass);
  vm->objectClass->metaclass = vm->classClass;
  vm->classClass->metaclass = vm->classClass;

  // The core class diagram ends up looking like this, where single lines point
  // to a class's superclass, and double lines point to its metaclass:
  //
  //             __________        /====\
  //            /          \      //    \\
  //           v            \     v      \\
  //     .---------.   .--------------.  //
  //     | Object  |==>|    Class     |==/
  //     '---------'   '--------------'
  //          ^               ^
  //          |               |
  //     .---------.   .--------------.   \
  //     |  Base   |==>|  Base.type   |    |
  //     '---------'   '--------------'    |
  //          ^               ^            | Hypothetical example classes
  //          |               |            |
  //     .---------.   .--------------.    |
  //     | Derived |==>| Derived.type |    |
  //     '---------'   '--------------'    /

  // The rest of the classes can not be defined normally.
  vm->boolClass = defineClass(vm, "Bool");
  NATIVE(vm->boolClass, "toString", bool_toString);
  NATIVE(vm->boolClass, "!", bool_not);

  vm->fiberClass = defineClass(vm, "Fiber");
  
  vm->fnClass = defineClass(vm, "Function");
  NATIVE(vm->fnClass, "call", fn_call);
  NATIVE(vm->fnClass, "call ", fn_call);
  NATIVE(vm->fnClass, "call  ", fn_call);
  NATIVE(vm->fnClass, "call   ", fn_call);
  NATIVE(vm->fnClass, "call    ", fn_call);
  NATIVE(vm->fnClass, "call     ", fn_call);
  NATIVE(vm->fnClass, "call      ", fn_call);
  NATIVE(vm->fnClass, "call       ", fn_call);
  NATIVE(vm->fnClass, "call        ", fn_call);
  NATIVE(vm->fnClass, "call         ", fn_call);
  NATIVE(vm->fnClass, "call          ", fn_call);
  NATIVE(vm->fnClass, "call           ", fn_call);
  NATIVE(vm->fnClass, "call            ", fn_call);
  NATIVE(vm->fnClass, "call             ", fn_call);
  NATIVE(vm->fnClass, "call              ", fn_call);
  NATIVE(vm->fnClass, "call               ", fn_call);
  NATIVE(vm->fnClass, "call                ", fn_call);

  vm->nullClass = defineClass(vm, "Null");
  NATIVE(vm->nullClass, "toString", null_toString);

  vm->stringClass = defineClass(vm, "String");
  NATIVE(vm->stringClass, "contains ", string_contains);
  NATIVE(vm->stringClass, "count", string_count);
  NATIVE(vm->stringClass, "toString", string_toString)
  NATIVE(vm->stringClass, "+ ", string_plus);
  NATIVE(vm->stringClass, "== ", string_eqeq);
  NATIVE(vm->stringClass, "!= ", string_bangeq);
  NATIVE(vm->stringClass, "[ ]", string_subscript);

  ObjClass* osClass = defineClass(vm, "OS");
  NATIVE(osClass->metaclass, "clock", os_clock);

  wrenInterpret(vm, "Wren core library", coreLibSource);

  vm->listClass = AS_CLASS(findGlobal(vm, "List"));
  NATIVE(vm->listClass, "add ", list_add);
  NATIVE(vm->listClass, "clear", list_clear);
  NATIVE(vm->listClass, "count", list_count);
  NATIVE(vm->listClass, "insert  ", list_insert);
  NATIVE(vm->listClass, "iterate ", list_iterate);
  NATIVE(vm->listClass, "iteratorValue ", list_iteratorValue);
  NATIVE(vm->listClass, "removeAt ", list_removeAt);
  NATIVE(vm->listClass, "[ ]", list_subscript);
  NATIVE(vm->listClass, "[ ]=", list_subscriptSetter);

  vm->numClass = AS_CLASS(findGlobal(vm, "Num"));
  NATIVE(vm->numClass, "abs", num_abs);
  NATIVE(vm->numClass, "toString", num_toString)
  NATIVE(vm->numClass, "-", num_negate);
  NATIVE(vm->numClass, "- ", num_minus);
  NATIVE(vm->numClass, "+ ", num_plus);
  NATIVE(vm->numClass, "* ", num_multiply);
  NATIVE(vm->numClass, "/ ", num_divide);
  NATIVE(vm->numClass, "% ", num_mod);
  NATIVE(vm->numClass, "< ", num_lt);
  NATIVE(vm->numClass, "> ", num_gt);
  NATIVE(vm->numClass, "<= ", num_lte);
  NATIVE(vm->numClass, ">= ", num_gte);
  NATIVE(vm->numClass, "~", num_bitwiseNot);

  // These are defined just so that 0 and -0 are equal, which is specified by
  // IEEE 754 even though they have different bit representations.
  NATIVE(vm->numClass, "== ", num_eqeq);
  NATIVE(vm->numClass, "!= ", num_bangeq);

  ObjClass* ioClass = AS_CLASS(findGlobal(vm, "IO"));
  NATIVE(ioClass->metaclass, "write__native__ ", io_writeString);
}
