#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wren_common.h"
#include "wren_core.h"
#include "wren_value.h"

// Binds a primitive method named [name] (in Wren) implemented using C function
// [fn] to `ObjClass` [cls].
#define PRIMITIVE(cls, name, function) \
    { \
      int symbol = wrenSymbolTableEnsure(vm, \
          &vm->methodNames, name, strlen(name)); \
      Method method; \
      method.type = METHOD_PRIMITIVE; \
      method.fn.primitive = prim_##function; \
      wrenBindMethod(vm, cls, symbol, method); \
    }

// Defines a primitive method whose C function name is [name]. This abstracts
// the actual type signature of a primitive function and makes it clear which C
// functions are invoked as primitives.
#define DEF_PRIMITIVE(name) \
    static PrimitiveResult prim_##name(WrenVM* vm, ObjFiber* fiber, Value* args)

#define RETURN_VAL(value)   do { args[0] = value; return PRIM_VALUE; } while (0)

#define RETURN_OBJ(obj)     RETURN_VAL(OBJ_VAL(obj))
#define RETURN_BOOL(value)  RETURN_VAL(BOOL_VAL(value))
#define RETURN_FALSE        RETURN_VAL(FALSE_VAL)
#define RETURN_NULL         RETURN_VAL(NULL_VAL)
#define RETURN_NUM(value)   RETURN_VAL(NUM_VAL(value))
#define RETURN_TRUE         RETURN_VAL(TRUE_VAL)

#define RETURN_ERROR(msg) \
    do { \
      args[0] = wrenStringFormat(vm, "$", msg); \
      return PRIM_ERROR; \
    } while (0);

// This string literal is generated automatically from core. Do not edit.
static const char* libSource =
"class Sequence {\n"
"  contains(element) {\n"
"    for (item in this) {\n"
"      if (element == item) {\n"
"        return true\n"
"      }\n"
"    }\n"
"    return false\n"
"  }\n"
"\n"
"  count {\n"
"    var result = 0\n"
"    for (element in this) {\n"
"      result = result + 1\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  count(f) {\n"
"    var result = 0\n"
"    for (element in this) {\n"
"      if (f.call(element)) {\n"
"        result = result + 1\n"
"      }\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  map(f) {\n"
"    var result = new List\n"
"    for (element in this) {\n"
"      result.add(f.call(element))\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  where(f) {\n"
"    var result = new List\n"
"    for (element in this) {\n"
"      if (f.call(element)) result.add(element)\n"
"    }\n"
"    return result\n"
"  }\n"
"\n"
"  all(f) {\n"
"    for (element in this) {\n"
"      if (!f.call(element)) return false\n"
"    }\n"
"    return true\n"
"  }\n"
"\n"
"  any(f) {\n"
"    for (element in this) {\n"
"      if (f.call(element)) return true\n"
"    }\n"
"    return false\n"
"  }\n"
"\n"
"  reduce(acc, f) {\n"
"    for (element in this) {\n"
"      acc = f.call(acc, element)\n"
"    }\n"
"    return acc\n"
"  }\n"
"\n"
"  reduce(f) {\n"
"    var iter = iterate(null)\n"
"    if (!iter) Fiber.abort(\"Can't reduce an empty sequence.\")\n"
"\n"
"    // Seed with the first element.\n"
"    var result = iteratorValue(iter)\n"
"    while (iter = iterate(iter)) {\n"
"      result = f.call(result, iteratorValue(iter))\n"
"    }\n"
"\n"
"    return result\n"
"  }\n"
"\n"
"  join { join(\"\") }\n"
"\n"
"  join(sep) {\n"
"    var first = true\n"
"    var result = \"\"\n"
"\n"
"    for (element in this) {\n"
"      if (!first) result = result + sep\n"
"      first = false\n"
"      result = result + element.toString\n"
"    }\n"
"\n"
"    return result\n"
"  }\n"
"}\n"
"\n"
"class String is Sequence {}\n"
"\n"
"class List is Sequence {\n"
"  addAll(other) {\n"
"    for (element in other) {\n"
"      add(element)\n"
"    }\n"
"    return other\n"
"  }\n"
"\n"
"  toString { \"[\" + join(\", \") + \"]\" }\n"
"\n"
"  +(other) {\n"
"    var result = this[0..-1]\n"
"    for (element in other) {\n"
"      result.add(element)\n"
"    }\n"
"    return result\n"
"  }\n"
"}\n"
"\n"
"class Map {\n"
"  keys { new MapKeySequence(this) }\n"
"  values { new MapValueSequence(this) }\n"
"\n"
"  toString {\n"
"    var first = true\n"
"    var result = \"{\"\n"
"\n"
"    for (key in keys) {\n"
"      if (!first) result = result + \", \"\n"
"      first = false\n"
"      result = result + key.toString + \": \" + this[key].toString\n"
"    }\n"
"\n"
"    return result + \"}\"\n"
"  }\n"
"}\n"
"\n"
"class MapKeySequence is Sequence {\n"
"  new(map) {\n"
"    _map = map\n"
"  }\n"
"\n"
"  iterate(n) { _map.iterate_(n) }\n"
"  iteratorValue(iterator) { _map.keyIteratorValue_(iterator) }\n"
"}\n"
"\n"
"class MapValueSequence is Sequence {\n"
"  new(map) {\n"
"    _map = map\n"
"  }\n"
"\n"
"  iterate(n) { _map.iterate_(n) }\n"
"  iteratorValue(iterator) { _map.valueIteratorValue_(iterator) }\n"
"}\n"
"\n"
"class Range is Sequence {}\n";

// Validates that the given argument in [args] is a function. Returns true if
// it is. If not, reports an error and returns false.
static bool validateFn(WrenVM* vm, Value* args, int index, const char* argName)
{
  if (IS_FN(args[index]) || IS_CLOSURE(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a function.", argName);
  return false;
}

// Validates that the given argument in [args] is a Num. Returns true if it is.
// If not, reports an error and returns false.
static bool validateNum(WrenVM* vm, Value* args, int index, const char* argName)
{
  if (IS_NUM(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a number.", argName);
  return false;
}

// Validates that [value] is an integer. Returns true if it is. If not, reports
// an error and returns false.
static bool validateIntValue(WrenVM* vm, Value* args, double value,
                             const char* argName)
{
  if (trunc(value) == value) return true;

  args[0] = wrenStringFormat(vm, "$ must be an integer.", argName);
  return false;
}

// Validates that the given argument in [args] is an integer. Returns true if
// it is. If not, reports an error and returns false.
static bool validateInt(WrenVM* vm, Value* args, int index, const char* argName)
{
  // Make sure it's a number first.
  if (!validateNum(vm, args, index, argName)) return false;

  return validateIntValue(vm, args, AS_NUM(args[index]), argName);
}

// Validates that [value] is an integer within `[0, count)`. Also allows
// negative indices which map backwards from the end. Returns the valid positive
// index value. If invalid, reports an error and returns `UINT32_MAX`.
static uint32_t validateIndexValue(WrenVM* vm, Value* args, uint32_t count,
                                    double value, const char* argName)
{
  if (!validateIntValue(vm, args, value, argName)) return UINT32_MAX;

  // Negative indices count from the end.
  if (value < 0) value = count + value;

  // Check bounds.
  if (value >= 0 && value < count) return (uint32_t)value;

  args[0] = wrenStringFormat(vm, "$ out of bounds.", argName);
  return UINT32_MAX;
}

// Validates that [key] is a valid object for use as a map key. Returns true if
// it is. If not, reports an error and returns false.
static bool validateKey(WrenVM* vm, Value* args, int index)
{
  Value arg = args[index];
  if (IS_BOOL(arg) || IS_CLASS(arg) || IS_NULL(arg) ||
      IS_NUM(arg) || IS_RANGE(arg) || IS_STRING(arg))
  {
    return true;
  }

  args[0] = CONST_STRING(vm, "Key must be a value type.");
  return false;
}

// Validates that the argument at [argIndex] is an integer within `[0, count)`.
// Also allows negative indices which map backwards from the end. Returns the
// valid positive index value. If invalid, reports an error and returns
// `UINT32_MAX`.
static uint32_t validateIndex(WrenVM* vm, Value* args, uint32_t count,
                              int arg, const char* argName)
{
  if (!validateNum(vm, args, arg, argName)) return UINT32_MAX;

  return validateIndexValue(vm, args, count, AS_NUM(args[arg]), argName);
}

// Validates that the given argument in [args] is a String. Returns true if it
// is. If not, reports an error and returns false.
static bool validateString(WrenVM* vm, Value* args, int index,
                           const char* argName)
{
  if (IS_STRING(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a string.", argName);
  return false;
}

// Given a [range] and the [length] of the object being operated on, determines
// the series of elements that should be chosen from the underlying object.
// Handles ranges that count backwards from the end as well as negative ranges.
//
// Returns the index from which the range should start or `UINT32_MAX` if the
// range is invalid. After calling, [length] will be updated with the number of
// elements in the resulting sequence. [step] will be direction that the range
// is going: `1` if the range is increasing from the start index or `-1` if the
// range is decreasing.
static uint32_t calculateRange(WrenVM* vm, Value* args, ObjRange* range,
                               uint32_t* length, int* step)
{
  *step = 0;
  
  // Corner case: an empty range at zero is allowed on an empty sequence.
  // This way, list[0..-1] and list[0...list.count] can be used to copy a list
  // even when empty.
  if (*length == 0 && range->from == 0 &&
      range->to == (range->isInclusive ? -1 : 0)) {
    return 0;
  }

  uint32_t from = validateIndexValue(vm, args, *length, range->from,
                                     "Range start");
  if (from == UINT32_MAX) return UINT32_MAX;

  // Bounds check the end manually to handle exclusive ranges.
  double value = range->to;
  if (!validateIntValue(vm, args, value, "Range end")) return UINT32_MAX;

  // Negative indices count from the end.
  if (value < 0) value = *length + value;

  // Convert the exclusive range to an inclusive one.
  if (!range->isInclusive)
  {
    // An exclusive range with the same start and end points is empty.
    if (value == from)
    {
      *length = 0;
      return from;
    }

    // Shift the endpoint to make it inclusive, handling both increasing and
    // decreasing ranges.
    value += value >= from ? -1 : 1;
  }

  // Check bounds.
  if (value < 0 || value >= *length)
  {
    args[0] = CONST_STRING(vm, "Range end out of bounds.");
    return UINT32_MAX;
  }

  uint32_t to = (uint32_t)value;
  *length = abs(from - to) + 1;
  *step = from < to ? 1 : -1;
  return from;
}

DEF_PRIMITIVE(bool_not)
{
  RETURN_BOOL(!AS_BOOL(args[0]));
}

DEF_PRIMITIVE(bool_toString)
{
  if (AS_BOOL(args[0]))
  {
    RETURN_VAL(CONST_STRING(vm, "true"));
  }
  else
  {
    RETURN_VAL(CONST_STRING(vm, "false"));
  }
}

DEF_PRIMITIVE(class_instantiate)
{
  RETURN_VAL(wrenNewInstance(vm, AS_CLASS(args[0])));
}

DEF_PRIMITIVE(class_name)
{
  RETURN_OBJ(AS_CLASS(args[0])->name);
}

DEF_PRIMITIVE(class_supertype)
{
  ObjClass* classObj = AS_CLASS(args[0]);

  // Object has no superclass.
  if (classObj->superclass == NULL) RETURN_NULL;

  RETURN_OBJ(classObj->superclass);
}

DEF_PRIMITIVE(fiber_instantiate)
{
  // Return the Fiber class itself. When we then call "new" on it, it will
  // create the fiber.
  RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(fiber_new)
{
  if (!validateFn(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjFiber* newFiber = wrenNewFiber(vm, AS_OBJ(args[1]));

  // The compiler expects the first slot of a function to hold the receiver.
  // Since a fiber's stack is invoked directly, it doesn't have one, so put it
  // in here.
  // TODO: Is there a cleaner solution?
  // TODO: If we make growable stacks, make sure this grows it.
  newFiber->stack[0] = NULL_VAL;
  newFiber->stackTop++;

  RETURN_OBJ(newFiber);
}

DEF_PRIMITIVE(fiber_abort)
{
  if (!validateString(vm, args, 1, "Error message")) return PRIM_ERROR;

  // Move the error message to the return position.
  args[0] = args[1];
  return PRIM_ERROR;
}

DEF_PRIMITIVE(fiber_call)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);

  if (runFiber->numFrames == 0) RETURN_ERROR("Cannot call a finished fiber.");
  if (runFiber->caller != NULL) RETURN_ERROR("Fiber has already been called.");

  // Remember who ran it.
  runFiber->caller = fiber;

  // If the fiber was yielded, make the yield call return null.
  if (runFiber->stackTop > runFiber->stack)
  {
    *(runFiber->stackTop - 1) = NULL_VAL;
  }

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_call1)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);

  if (runFiber->numFrames == 0) RETURN_ERROR("Cannot call a finished fiber.");
  if (runFiber->caller != NULL) RETURN_ERROR("Fiber has already been called.");

  // Remember who ran it.
  runFiber->caller = fiber;

  // If the fiber was yielded, make the yield call return the value passed to
  // run.
  if (runFiber->stackTop > runFiber->stack)
  {
    *(runFiber->stackTop - 1) = args[1];
  }

  // When the calling fiber resumes, we'll store the result of the run call
  // in its stack. Since fiber.run(value) has two arguments (the fiber and the
  // value) and we only need one slot for the result, discard the other slot
  // now.
  fiber->stackTop--;

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_current)
{
  RETURN_OBJ(fiber);
}

DEF_PRIMITIVE(fiber_error)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);
  if (runFiber->error == NULL) RETURN_NULL;
  RETURN_OBJ(runFiber->error);
}

DEF_PRIMITIVE(fiber_isDone)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);
  RETURN_BOOL(runFiber->numFrames == 0 || runFiber->error != NULL);
}

DEF_PRIMITIVE(fiber_run)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);

  if (runFiber->numFrames == 0) RETURN_ERROR("Cannot run a finished fiber.");

  // If the fiber was yielded, make the yield call return null.
  if (runFiber->caller == NULL && runFiber->stackTop > runFiber->stack)
  {
    *(runFiber->stackTop - 1) = NULL_VAL;
  }

  // Unlike run, this does not remember the calling fiber. Instead, it
  // remember's *that* fiber's caller. You can think of it like tail call
  // elimination. The switched-from fiber is discarded and when the switched
  // to fiber completes or yields, control passes to the switched-from fiber's
  // caller.
  runFiber->caller = fiber->caller;

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_run1)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);

  if (runFiber->numFrames == 0) RETURN_ERROR("Cannot run a finished fiber.");

  // If the fiber was yielded, make the yield call return the value passed to
  // run.
  if (runFiber->caller == NULL && runFiber->stackTop > runFiber->stack)
  {
    *(runFiber->stackTop - 1) = args[1];
  }

  // Unlike run, this does not remember the calling fiber. Instead, it
  // remember's *that* fiber's caller. You can think of it like tail call
  // elimination. The switched-from fiber is discarded and when the switched
  // to fiber completes or yields, control passes to the switched-from fiber's
  // caller.
  runFiber->caller = fiber->caller;

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_try)
{
  ObjFiber* runFiber = AS_FIBER(args[0]);

  if (runFiber->numFrames == 0) RETURN_ERROR("Cannot try a finished fiber.");
  if (runFiber->caller != NULL) RETURN_ERROR("Fiber has already been called.");

  // Remember who ran it.
  runFiber->caller = fiber;
  runFiber->callerIsTrying = true;

  // If the fiber was yielded, make the yield call return null.
  if (runFiber->stackTop > runFiber->stack)
  {
    *(runFiber->stackTop - 1) = NULL_VAL;
  }

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_yield)
{
  // Unhook this fiber from the one that called it.
  ObjFiber* caller = fiber->caller;
  fiber->caller = NULL;
  fiber->callerIsTrying = false;

  // If we don't have any other pending fibers, jump all the way out of the
  // interpreter.
  if (caller == NULL)
  {
    args[0] = NULL_VAL;
  }
  else
  {
    // Make the caller's run method return null.
    *(caller->stackTop - 1) = NULL_VAL;

    // Return the fiber to resume.
    args[0] = OBJ_VAL(caller);
  }

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fiber_yield1)
{
  // Unhook this fiber from the one that called it.
  ObjFiber* caller = fiber->caller;
  fiber->caller = NULL;
  fiber->callerIsTrying = false;

  // If we don't have any other pending fibers, jump all the way out of the
  // interpreter.
  if (caller == NULL)
  {
    args[0] = NULL_VAL;
  }
  else
  {
    // Make the caller's run method return the argument passed to yield.
    *(caller->stackTop - 1) = args[1];

    // When the yielding fiber resumes, we'll store the result of the yield call
    // in its stack. Since Fiber.yield(value) has two arguments (the Fiber class
    // and the value) and we only need one slot for the result, discard the other
    // slot now.
    fiber->stackTop--;

    // Return the fiber to resume.
    args[0] = OBJ_VAL(caller);
  }

  return PRIM_RUN_FIBER;
}

DEF_PRIMITIVE(fn_instantiate)
{
  // Return the Fn class itself. When we then call "new" on it, it will return
  // the block.
  RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(fn_new)
{
  if (!validateFn(vm, args, 1, "Argument")) return PRIM_ERROR;

  // The block argument is already a function, so just return it.
  RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(fn_arity)
{
  RETURN_NUM(AS_FN(args[0])->arity);
}

static PrimitiveResult callFunction(WrenVM* vm, Value* args, int numArgs)
{
  ObjFn* fn;
  if (IS_CLOSURE(args[0]))
  {
    fn = AS_CLOSURE(args[0])->fn;
  }
  else
  {
    fn = AS_FN(args[0]);
  }

  if (numArgs < fn->arity) RETURN_ERROR("Function expects more arguments.");

  return PRIM_CALL;
}

DEF_PRIMITIVE(fn_call0) { return callFunction(vm, args, 0); }
DEF_PRIMITIVE(fn_call1) { return callFunction(vm, args, 1); }
DEF_PRIMITIVE(fn_call2) { return callFunction(vm, args, 2); }
DEF_PRIMITIVE(fn_call3) { return callFunction(vm, args, 3); }
DEF_PRIMITIVE(fn_call4) { return callFunction(vm, args, 4); }
DEF_PRIMITIVE(fn_call5) { return callFunction(vm, args, 5); }
DEF_PRIMITIVE(fn_call6) { return callFunction(vm, args, 6); }
DEF_PRIMITIVE(fn_call7) { return callFunction(vm, args, 7); }
DEF_PRIMITIVE(fn_call8) { return callFunction(vm, args, 8); }
DEF_PRIMITIVE(fn_call9) { return callFunction(vm, args, 9); }
DEF_PRIMITIVE(fn_call10) { return callFunction(vm, args, 10); }
DEF_PRIMITIVE(fn_call11) { return callFunction(vm, args, 11); }
DEF_PRIMITIVE(fn_call12) { return callFunction(vm, args, 12); }
DEF_PRIMITIVE(fn_call13) { return callFunction(vm, args, 13); }
DEF_PRIMITIVE(fn_call14) { return callFunction(vm, args, 14); }
DEF_PRIMITIVE(fn_call15) { return callFunction(vm, args, 15); }
DEF_PRIMITIVE(fn_call16) { return callFunction(vm, args, 16); }

DEF_PRIMITIVE(fn_toString)
{
  RETURN_VAL(CONST_STRING(vm, "<fn>"));
}

DEF_PRIMITIVE(list_instantiate)
{
  RETURN_OBJ(wrenNewList(vm, 0));
}

DEF_PRIMITIVE(list_add)
{
  wrenValueBufferWrite(vm, &AS_LIST(args[0])->elements, args[1]);
  RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(list_clear)
{
  wrenValueBufferClear(vm, &AS_LIST(args[0])->elements);
  RETURN_NULL;
}

DEF_PRIMITIVE(list_count)
{
  RETURN_NUM(AS_LIST(args[0])->elements.count);
}

DEF_PRIMITIVE(list_insert)
{
  ObjList* list = AS_LIST(args[0]);

  // count + 1 here so you can "insert" at the very end.
  uint32_t index = validateIndex(vm, args, list->elements.count + 1, 1,
                                 "Index");
  if (index == UINT32_MAX) return PRIM_ERROR;

  wrenListInsert(vm, list, args[2], index);
  RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(list_iterate)
{
  ObjList* list = AS_LIST(args[0]);

  // If we're starting the iteration, return the first index.
  if (IS_NULL(args[1]))
  {
    if (list->elements.count == 0) RETURN_FALSE;
    RETURN_NUM(0);
  }

  if (!validateInt(vm, args, 1, "Iterator")) return PRIM_ERROR;

  // Stop if we're out of bounds.
  double index = AS_NUM(args[1]);
  if (index < 0 || index >= list->elements.count - 1) RETURN_FALSE;

  // Otherwise, move to the next index.
  RETURN_NUM(index + 1);
}

DEF_PRIMITIVE(list_iteratorValue)
{
  ObjList* list = AS_LIST(args[0]);
  uint32_t index = validateIndex(vm, args, list->elements.count, 1, "Iterator");
  if (index == UINT32_MAX) return PRIM_ERROR;

  RETURN_VAL(list->elements.data[index]);
}

DEF_PRIMITIVE(list_removeAt)
{
  ObjList* list = AS_LIST(args[0]);
  uint32_t index = validateIndex(vm, args, list->elements.count, 1, "Index");
  if (index == UINT32_MAX) return PRIM_ERROR;

  RETURN_VAL(wrenListRemoveAt(vm, list, index));
}

DEF_PRIMITIVE(list_subscript)
{
  ObjList* list = AS_LIST(args[0]);

  if (IS_NUM(args[1]))
  {
    uint32_t index = validateIndex(vm, args, list->elements.count, 1,
                                   "Subscript");
    if (index == UINT32_MAX) return PRIM_ERROR;

    RETURN_VAL(list->elements.data[index]);
  }

  if (!IS_RANGE(args[1]))
  {
    RETURN_ERROR("Subscript must be a number or a range.");
  }

  int step;
  uint32_t count = list->elements.count;
  uint32_t start = calculateRange(vm, args, AS_RANGE(args[1]), &count, &step);
  if (start == UINT32_MAX) return PRIM_ERROR;

  ObjList* result = wrenNewList(vm, count);
  for (uint32_t i = 0; i < count; i++)
  {
    result->elements.data[i] = list->elements.data[start + (i * step)];
  }

  RETURN_OBJ(result);
}

DEF_PRIMITIVE(list_subscriptSetter)
{
  ObjList* list = AS_LIST(args[0]);
  uint32_t index = validateIndex(vm, args, list->elements.count, 1,
                                 "Subscript");
  if (index == UINT32_MAX) return PRIM_ERROR;

  list->elements.data[index] = args[2];
  RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(map_instantiate)
{
  RETURN_OBJ(wrenNewMap(vm));
}

DEF_PRIMITIVE(map_subscript)
{
  if (!validateKey(vm, args, 1)) return PRIM_ERROR;

  ObjMap* map = AS_MAP(args[0]);
  Value value = wrenMapGet(map, args[1]);
  if (IS_UNDEFINED(value)) RETURN_NULL;

  RETURN_VAL(value);
}

DEF_PRIMITIVE(map_subscriptSetter)
{
  if (!validateKey(vm, args, 1)) return PRIM_ERROR;

  wrenMapSet(vm, AS_MAP(args[0]), args[1], args[2]);
  RETURN_VAL(args[2]);
}

DEF_PRIMITIVE(map_clear)
{
  wrenMapClear(vm, AS_MAP(args[0]));
  RETURN_NULL;
}

DEF_PRIMITIVE(map_containsKey)
{
  if (!validateKey(vm, args, 1)) return PRIM_ERROR;

  RETURN_BOOL(!IS_UNDEFINED(wrenMapGet(AS_MAP(args[0]), args[1])));
}

DEF_PRIMITIVE(map_count)
{
  RETURN_NUM(AS_MAP(args[0])->count);
}

DEF_PRIMITIVE(map_iterate)
{
  ObjMap* map = AS_MAP(args[0]);

  if (map->count == 0) RETURN_FALSE;

  // If we're starting the iteration, start at the first used entry.
  uint32_t index = 0;

  // Otherwise, start one past the last entry we stopped at.
  if (!IS_NULL(args[1]))
  {
    if (!validateInt(vm, args, 1, "Iterator")) return PRIM_ERROR;

    if (AS_NUM(args[1]) < 0) RETURN_FALSE;
    index = (uint32_t)AS_NUM(args[1]);

    if (index >= map->capacity) RETURN_FALSE;

    // Advance the iterator.
    index++;
  }

  // Find a used entry, if any.
  for (; index < map->capacity; index++)
  {
    if (!IS_UNDEFINED(map->entries[index].key)) RETURN_NUM(index);
  }

  // If we get here, walked all of the entries.
  RETURN_FALSE;
}

DEF_PRIMITIVE(map_remove)
{
  if (!validateKey(vm, args, 1)) return PRIM_ERROR;

  RETURN_VAL(wrenMapRemoveKey(vm, AS_MAP(args[0]), args[1]));
}

DEF_PRIMITIVE(map_keyIteratorValue)
{
  ObjMap* map = AS_MAP(args[0]);
  uint32_t index = validateIndex(vm, args, map->capacity, 1, "Iterator");
  if (index == UINT32_MAX) return PRIM_ERROR;

  MapEntry* entry = &map->entries[index];
  if (IS_UNDEFINED(entry->key))
  {
    RETURN_ERROR("Invalid map iterator value.");
  }

  RETURN_VAL(entry->key);
}

DEF_PRIMITIVE(map_valueIteratorValue)
{
  ObjMap* map = AS_MAP(args[0]);
  uint32_t index = validateIndex(vm, args, map->capacity, 1, "Iterator");
  if (index == UINT32_MAX) return PRIM_ERROR;

  MapEntry* entry = &map->entries[index];
  if (IS_UNDEFINED(entry->key))
  {
    RETURN_ERROR("Invalid map iterator value.");
  }

  RETURN_VAL(entry->value);
}

DEF_PRIMITIVE(null_not)
{
  RETURN_VAL(TRUE_VAL);
}

DEF_PRIMITIVE(null_toString)
{
  RETURN_VAL(CONST_STRING(vm, "null"));
}

DEF_PRIMITIVE(num_fromString)
{
  if (!validateString(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjString* string = AS_STRING(args[1]);

  // Corner case: Can't parse an empty string.
  if (string->length == 0) RETURN_NULL;

  errno = 0;
  char* end;
  double number = strtod(string->value, &end);

  // Skip past any trailing whitespace.
  while (*end != '\0' && isspace(*end)) end++;

  if (errno == ERANGE)
  {
    args[0] = CONST_STRING(vm, "Number literal is too large.");
    return PRIM_ERROR;
  }

  // We must have consumed the entire string. Otherwise, it contains non-number
  // characters and we can't parse it.
  if (end < string->value + string->length) RETURN_NULL;

  RETURN_NUM(number);
}

DEF_PRIMITIVE(num_pi)
{
  RETURN_NUM(3.14159265358979323846);
}

// Defines a primitive on Num that calls infix [op] and returns [type].
#define DEF_NUM_INFIX(name, op, type) \
    DEF_PRIMITIVE(num_##name) \
    { \
      if (!validateNum(vm, args, 1, "Right operand")) return PRIM_ERROR; \
      RETURN_##type(AS_NUM(args[0]) op AS_NUM(args[1])); \
    }

DEF_NUM_INFIX(minus,    -,  NUM)
DEF_NUM_INFIX(plus,     +,  NUM)
DEF_NUM_INFIX(multiply, *,  NUM)
DEF_NUM_INFIX(divide,   /,  NUM)
DEF_NUM_INFIX(lt,       <,  BOOL)
DEF_NUM_INFIX(gt,       >,  BOOL)
DEF_NUM_INFIX(lte,      <=, BOOL)
DEF_NUM_INFIX(gte,      >=, BOOL)

// Defines a primitive on Num that call infix bitwise [op].
#define DEF_NUM_BITWISE(name, op) \
    DEF_PRIMITIVE(num_bitwise##name) \
    { \
      if (!validateNum(vm, args, 1, "Right operand")) return PRIM_ERROR; \
      uint32_t left = (uint32_t)AS_NUM(args[0]); \
      uint32_t right = (uint32_t)AS_NUM(args[1]); \
      RETURN_NUM(left op right); \
    }

DEF_NUM_BITWISE(And,        &)
DEF_NUM_BITWISE(Or,         |)
DEF_NUM_BITWISE(Xor,        ^)
DEF_NUM_BITWISE(LeftShift,  <<)
DEF_NUM_BITWISE(RightShift, >>)

// Defines a primitive method on Num that returns the result of [fn].
#define DEF_NUM_FN(name, fn) \
    DEF_PRIMITIVE(num_##name) \
    { \
      RETURN_NUM(fn(AS_NUM(args[0]))); \
    }

DEF_NUM_FN(abs,     fabs)
DEF_NUM_FN(acos,    acos)
DEF_NUM_FN(asin,    asin)
DEF_NUM_FN(atan,    atan)
DEF_NUM_FN(ceil,    ceil)
DEF_NUM_FN(cos,     cos)
DEF_NUM_FN(floor,   floor)
DEF_NUM_FN(negate,  -)
DEF_NUM_FN(sin,     sin)
DEF_NUM_FN(sqrt,    sqrt)
DEF_NUM_FN(tan,     tan)

DEF_PRIMITIVE(num_mod)
{
  if (!validateNum(vm, args, 1, "Right operand")) return PRIM_ERROR;
  RETURN_NUM(fmod(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_eqeq)
{
  if (!IS_NUM(args[1])) RETURN_FALSE;
  RETURN_BOOL(AS_NUM(args[0]) == AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_bangeq)
{
  if (!IS_NUM(args[1])) RETURN_TRUE;
  RETURN_BOOL(AS_NUM(args[0]) != AS_NUM(args[1]));
}

DEF_PRIMITIVE(num_bitwiseNot)
{
  // Bitwise operators always work on 32-bit unsigned ints.
  RETURN_NUM(~(uint32_t)AS_NUM(args[0]));
}

DEF_PRIMITIVE(num_dotDot)
{
  if (!validateNum(vm, args, 1, "Right hand side of range")) return PRIM_ERROR;

  double from = AS_NUM(args[0]);
  double to = AS_NUM(args[1]);
  RETURN_VAL(wrenNewRange(vm, from, to, true));
}

DEF_PRIMITIVE(num_dotDotDot)
{
  if (!validateNum(vm, args, 1, "Right hand side of range")) return PRIM_ERROR;

  double from = AS_NUM(args[0]);
  double to = AS_NUM(args[1]);
  RETURN_VAL(wrenNewRange(vm, from, to, false));
}

DEF_PRIMITIVE(num_atan2)
{
  RETURN_NUM(atan2(AS_NUM(args[0]), AS_NUM(args[1])));
}

DEF_PRIMITIVE(num_fraction)
{
  double dummy;
  RETURN_NUM(modf(AS_NUM(args[0]) , &dummy));
}

DEF_PRIMITIVE(num_isNan)
{
  RETURN_BOOL(isnan(AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_sign)
{
  double value = AS_NUM(args[0]);
  if (value > 0)
  {
    RETURN_NUM(1);
  }
  else if (value < 0)
  {
    RETURN_NUM(-1);
  }
  else
  {
    RETURN_NUM(0);
  }
}

DEF_PRIMITIVE(num_toString)
{
  RETURN_VAL(wrenNumToString(vm, AS_NUM(args[0])));
}

DEF_PRIMITIVE(num_truncate)
{
  double integer;
  modf(AS_NUM(args[0]) , &integer);
  RETURN_NUM(integer);
}

DEF_PRIMITIVE(object_not)
{
  RETURN_VAL(FALSE_VAL);
}

DEF_PRIMITIVE(object_eqeq)
{
  RETURN_BOOL(wrenValuesEqual(args[0], args[1]));
}

DEF_PRIMITIVE(object_bangeq)
{
  RETURN_BOOL(!wrenValuesEqual(args[0], args[1]));
}

DEF_PRIMITIVE(object_new)
{
  // This is the default argument-less constructor that all objects inherit.
  // It just returns "this".
  RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(object_toString)
{
  if (IS_CLASS(args[0]))
  {
    RETURN_OBJ(AS_CLASS(args[0])->name);
  }
  else if (IS_INSTANCE(args[0]))
  {
    ObjInstance* instance = AS_INSTANCE(args[0]);
    Value name = OBJ_VAL(instance->obj.classObj->name);
    RETURN_VAL(wrenStringFormat(vm, "instance of @", name));
  }

  RETURN_VAL(CONST_STRING(vm, "<object>"));
}

DEF_PRIMITIVE(object_type)
{
  RETURN_OBJ(wrenGetClass(vm, args[0]));
}

DEF_PRIMITIVE(object_instantiate)
{
  RETURN_ERROR("Must provide a class to 'new' to construct.");
}

DEF_PRIMITIVE(range_from)
{
  RETURN_NUM(AS_RANGE(args[0])->from);
}

DEF_PRIMITIVE(range_to)
{
  RETURN_NUM(AS_RANGE(args[0])->to);
}

DEF_PRIMITIVE(range_min)
{
  ObjRange* range = AS_RANGE(args[0]);
  RETURN_NUM(fmin(range->from, range->to));
}

DEF_PRIMITIVE(range_max)
{
  ObjRange* range = AS_RANGE(args[0]);
  RETURN_NUM(fmax(range->from, range->to));
}

DEF_PRIMITIVE(range_isInclusive)
{
  RETURN_BOOL(AS_RANGE(args[0])->isInclusive);
}

DEF_PRIMITIVE(range_iterate)
{
  ObjRange* range = AS_RANGE(args[0]);

  // Special case: empty range.
  if (range->from == range->to && !range->isInclusive) RETURN_FALSE;

  // Start the iteration.
  if (IS_NULL(args[1])) RETURN_NUM(range->from);

  if (!validateNum(vm, args, 1, "Iterator")) return PRIM_ERROR;

  double iterator = AS_NUM(args[1]);

  // Iterate towards [to] from [from].
  if (range->from < range->to)
  {
    iterator++;
    if (iterator > range->to) RETURN_FALSE;
  }
  else
  {
    iterator--;
    if (iterator < range->to) RETURN_FALSE;
  }

  if (!range->isInclusive && iterator == range->to) RETURN_FALSE;

  RETURN_NUM(iterator);
}

DEF_PRIMITIVE(range_iteratorValue)
{
  // Assume the iterator is a number so that is the value of the range.
  RETURN_VAL(args[1]);
}

DEF_PRIMITIVE(range_toString)
{
  ObjRange* range = AS_RANGE(args[0]);

  Value from = wrenNumToString(vm, range->from);
  wrenPushRoot(vm, AS_OBJ(from));

  Value to = wrenNumToString(vm, range->to);
  wrenPushRoot(vm, AS_OBJ(to));

  Value result = wrenStringFormat(vm, "@$@", from,
                                  range->isInclusive ? ".." : "...", to);

  wrenPopRoot(vm);
  wrenPopRoot(vm);
  RETURN_VAL(result);
}

DEF_PRIMITIVE(string_fromCodePoint)
{
  if (!validateInt(vm, args, 1, "Code point")) return PRIM_ERROR;

  int codePoint = (int)AS_NUM(args[1]);
  if (codePoint < 0)
  {
    RETURN_ERROR("Code point cannot be negative.");
  }
  else if (codePoint > 0x10ffff)
  {
    RETURN_ERROR("Code point cannot be greater than 0x10ffff.");
  }

  RETURN_VAL(wrenStringFromCodePoint(vm, (int)AS_NUM(args[1])));
}

DEF_PRIMITIVE(string_contains)
{
  if (!validateString(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjString* string = AS_STRING(args[0]);
  ObjString* search = AS_STRING(args[1]);

  RETURN_BOOL(wrenStringFind(string, search) != UINT32_MAX);
}

DEF_PRIMITIVE(string_count)
{
  RETURN_NUM(AS_STRING(args[0])->length);
}

DEF_PRIMITIVE(string_endsWith)
{
  if (!validateString(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjString* string = AS_STRING(args[0]);
  ObjString* search = AS_STRING(args[1]);

  // Corner case, if the search string is longer than return false right away.
  if (search->length > string->length) RETURN_FALSE;

  RETURN_BOOL(memcmp(string->value + string->length - search->length,
                     search->value, search->length) == 0);
}

DEF_PRIMITIVE(string_indexOf)
{
  if (!validateString(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjString* string = AS_STRING(args[0]);
  ObjString* search = AS_STRING(args[1]);

  uint32_t index = wrenStringFind(string, search);
  RETURN_NUM(index == UINT32_MAX ? -1 : (int)index);
}

DEF_PRIMITIVE(string_iterate)
{
  ObjString* string = AS_STRING(args[0]);

  // If we're starting the iteration, return the first index.
  if (IS_NULL(args[1]))
  {
    if (string->length == 0) RETURN_FALSE;
    RETURN_NUM(0);
  }

  if (!validateInt(vm, args, 1, "Iterator")) return PRIM_ERROR;

  if (AS_NUM(args[1]) < 0) RETURN_FALSE;
  uint32_t index = (uint32_t)AS_NUM(args[1]);

  // Advance to the beginning of the next UTF-8 sequence.
  do
  {
    index++;
    if (index >= string->length) RETURN_FALSE;
  } while ((string->value[index] & 0xc0) == 0x80);

  RETURN_NUM(index);
}

DEF_PRIMITIVE(string_iteratorValue)
{
  ObjString* string = AS_STRING(args[0]);
  uint32_t index = validateIndex(vm, args, string->length, 1, "Iterator");
  if (index == UINT32_MAX) return PRIM_ERROR;

  RETURN_VAL(wrenStringCodePointAt(vm, string, index));
}

DEF_PRIMITIVE(string_startsWith)
{
  if (!validateString(vm, args, 1, "Argument")) return PRIM_ERROR;

  ObjString* string = AS_STRING(args[0]);
  ObjString* search = AS_STRING(args[1]);

  // Corner case, if the search string is longer than return false right away.
  if (search->length > string->length) RETURN_FALSE;

  RETURN_BOOL(memcmp(string->value, search->value, search->length) == 0);
}

DEF_PRIMITIVE(string_toString)
{
  RETURN_VAL(args[0]);
}

DEF_PRIMITIVE(string_plus)
{
  if (!validateString(vm, args, 1, "Right operand")) return PRIM_ERROR;
  RETURN_VAL(wrenStringFormat(vm, "@@", args[0], args[1]));
}

DEF_PRIMITIVE(string_subscript)
{
  ObjString* string = AS_STRING(args[0]);

  if (IS_NUM(args[1]))
  {
    int index = validateIndex(vm, args, string->length, 1, "Subscript");
    if (index == -1) return PRIM_ERROR;

    RETURN_VAL(wrenStringCodePointAt(vm, string, index));
  }

  if (!IS_RANGE(args[1]))
  {
    RETURN_ERROR("Subscript must be a number or a range.");
  }

  // TODO: Handle UTF-8 here.
  /*
  int step;
  int count = string->length;
  int start = calculateRange(vm, args, AS_RANGE(args[1]), &count, &step);
  if (start == -1) return PRIM_ERROR;

  ObjString* result = wrenNewUninitializedString(vm, count);
  for (int i = 0; i < count; i++)
  {
    result->value[i] = string->value[start + (i * step)];
  }
  result->value[count] = '\0';

  RETURN_OBJ(result);
  */
  RETURN_ERROR("Subscript ranges for strings are not implemented yet.");
}

static ObjClass* defineSingleClass(WrenVM* vm, const char* name)
{
  ObjString* nameString = AS_STRING(wrenStringFormat(vm, "$", name));
  wrenPushRoot(vm, (Obj*)nameString);

  ObjClass* classObj = wrenNewSingleClass(vm, 0, nameString);
  wrenDefineVariable(vm, NULL, name, nameString->length, OBJ_VAL(classObj));

  wrenPopRoot(vm);
  return classObj;
}

static ObjClass* defineClass(WrenVM* vm, const char* name)
{
  ObjString* nameString = AS_STRING(wrenStringFormat(vm, "$", name));
  wrenPushRoot(vm, (Obj*)nameString);

  ObjClass* classObj = wrenNewClass(vm, vm->objectClass, 0, nameString);
  wrenDefineVariable(vm, NULL, name, nameString->length, OBJ_VAL(classObj));

  wrenPopRoot(vm);
  return classObj;
}

void wrenInitializeCore(WrenVM* vm)
{
  // Define the root Object class. This has to be done a little specially
  // because it has no superclass and an unusual metaclass (Class).
  vm->objectClass = defineSingleClass(vm, "Object");
  PRIMITIVE(vm->objectClass, "!", object_not);
  PRIMITIVE(vm->objectClass, "==(_)", object_eqeq);
  PRIMITIVE(vm->objectClass, "!=(_)", object_bangeq);
  PRIMITIVE(vm->objectClass, "new", object_new);
  PRIMITIVE(vm->objectClass, "toString", object_toString);
  PRIMITIVE(vm->objectClass, "type", object_type);
  PRIMITIVE(vm->objectClass, "<instantiate>", object_instantiate);

  // Now we can define Class, which is a subclass of Object, but Object's
  // metaclass.
  vm->classClass = defineSingleClass(vm, "Class");

  // Now that Object and Class are defined, we can wire them up to each other.
  wrenBindSuperclass(vm, vm->classClass, vm->objectClass);
  vm->objectClass->obj.classObj = vm->classClass;
  vm->classClass->obj.classObj = vm->classClass;

  // Define the methods specific to Class after wiring up its superclass to
  // prevent the inherited ones from overwriting them.
  PRIMITIVE(vm->classClass, "<instantiate>", class_instantiate);
  PRIMITIVE(vm->classClass, "name", class_name);
  PRIMITIVE(vm->classClass, "supertype", class_supertype);

  // The core class diagram ends up looking like this, where single lines point
  // to a class's superclass, and double lines point to its metaclass:
  //
  //           .------------.    .========.
  //           |            |    ||      ||
  //           v            |    v       ||
  //     .---------.   .--------------.  ||
  //     | Object  |==>|    Class     |==='
  //     '---------'   '--------------'
  //          ^               ^
  //          |               |
  //     .---------.   .--------------.   -.
  //     |  Base   |==>|  Base.type   |    |
  //     '---------'   '--------------'    |
  //          ^               ^            | Hypothetical example classes
  //          |               |            |
  //     .---------.   .--------------.    |
  //     | Derived |==>| Derived.type |    |
  //     '---------'   '--------------'   -'

  // The rest of the classes can not be defined normally.
  vm->boolClass = defineClass(vm, "Bool");
  PRIMITIVE(vm->boolClass, "toString", bool_toString);
  PRIMITIVE(vm->boolClass, "!", bool_not);

  vm->fiberClass = defineClass(vm, "Fiber");
  PRIMITIVE(vm->fiberClass->obj.classObj, "<instantiate>", fiber_instantiate);
  PRIMITIVE(vm->fiberClass->obj.classObj, "new(_)", fiber_new);
  PRIMITIVE(vm->fiberClass->obj.classObj, "abort(_)", fiber_abort);
  PRIMITIVE(vm->fiberClass->obj.classObj, "current", fiber_current);
  PRIMITIVE(vm->fiberClass->obj.classObj, "yield()", fiber_yield);
  PRIMITIVE(vm->fiberClass->obj.classObj, "yield(_)", fiber_yield1);
  PRIMITIVE(vm->fiberClass, "call()", fiber_call);
  PRIMITIVE(vm->fiberClass, "call(_)", fiber_call1);
  PRIMITIVE(vm->fiberClass, "error", fiber_error);
  PRIMITIVE(vm->fiberClass, "isDone", fiber_isDone);
  PRIMITIVE(vm->fiberClass, "run()", fiber_run);
  PRIMITIVE(vm->fiberClass, "run(_)", fiber_run1);
  PRIMITIVE(vm->fiberClass, "try()", fiber_try);

  vm->fnClass = defineClass(vm, "Fn");

  PRIMITIVE(vm->fnClass->obj.classObj, "<instantiate>", fn_instantiate);
  PRIMITIVE(vm->fnClass->obj.classObj, "new(_)", fn_new);

  PRIMITIVE(vm->fnClass, "arity", fn_arity);
  PRIMITIVE(vm->fnClass, "call()", fn_call0);
  PRIMITIVE(vm->fnClass, "call(_)", fn_call1);
  PRIMITIVE(vm->fnClass, "call(_,_)", fn_call2);
  PRIMITIVE(vm->fnClass, "call(_,_,_)", fn_call3);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_)", fn_call4);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_)", fn_call5);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_)", fn_call6);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_)", fn_call7);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_)", fn_call8);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_)", fn_call9);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_)", fn_call10);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_)", fn_call11);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_)", fn_call12);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call13);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call14);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call15);
  PRIMITIVE(vm->fnClass, "call(_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_)", fn_call16);
  PRIMITIVE(vm->fnClass, "toString", fn_toString);

  vm->nullClass = defineClass(vm, "Null");
  PRIMITIVE(vm->nullClass, "!", null_not);
  PRIMITIVE(vm->nullClass, "toString", null_toString);

  vm->numClass = defineClass(vm, "Num");
  PRIMITIVE(vm->numClass->obj.classObj, "fromString(_)", num_fromString);
  PRIMITIVE(vm->numClass->obj.classObj, "pi", num_pi);
  PRIMITIVE(vm->numClass, "-(_)", num_minus);
  PRIMITIVE(vm->numClass, "+(_)", num_plus);
  PRIMITIVE(vm->numClass, "*(_)", num_multiply);
  PRIMITIVE(vm->numClass, "/(_)", num_divide);
  PRIMITIVE(vm->numClass, "<(_)", num_lt);
  PRIMITIVE(vm->numClass, ">(_)", num_gt);
  PRIMITIVE(vm->numClass, "<=(_)", num_lte);
  PRIMITIVE(vm->numClass, ">=(_)", num_gte);
  PRIMITIVE(vm->numClass, "&(_)", num_bitwiseAnd);
  PRIMITIVE(vm->numClass, "|(_)", num_bitwiseOr);
  PRIMITIVE(vm->numClass, "^(_)", num_bitwiseXor);
  PRIMITIVE(vm->numClass, "<<(_)", num_bitwiseLeftShift);
  PRIMITIVE(vm->numClass, ">>(_)", num_bitwiseRightShift);
  PRIMITIVE(vm->numClass, "abs", num_abs);
  PRIMITIVE(vm->numClass, "acos", num_acos);
  PRIMITIVE(vm->numClass, "asin", num_asin);
  PRIMITIVE(vm->numClass, "atan", num_atan);
  PRIMITIVE(vm->numClass, "ceil", num_ceil);
  PRIMITIVE(vm->numClass, "cos", num_cos);
  PRIMITIVE(vm->numClass, "floor", num_floor);
  PRIMITIVE(vm->numClass, "-", num_negate);
  PRIMITIVE(vm->numClass, "sin", num_sin);
  PRIMITIVE(vm->numClass, "sqrt", num_sqrt);
  PRIMITIVE(vm->numClass, "tan", num_tan);
  PRIMITIVE(vm->numClass, "%(_)", num_mod);
  PRIMITIVE(vm->numClass, "~", num_bitwiseNot);
  PRIMITIVE(vm->numClass, "..(_)", num_dotDot);
  PRIMITIVE(vm->numClass, "...(_)", num_dotDotDot);
  PRIMITIVE(vm->numClass, "atan(_)", num_atan2);
  PRIMITIVE(vm->numClass, "fraction", num_fraction);
  PRIMITIVE(vm->numClass, "isNan", num_isNan);
  PRIMITIVE(vm->numClass, "sign", num_sign);
  PRIMITIVE(vm->numClass, "toString", num_toString);
  PRIMITIVE(vm->numClass, "truncate", num_truncate);

  // These are defined just so that 0 and -0 are equal, which is specified by
  // IEEE 754 even though they have different bit representations.
  PRIMITIVE(vm->numClass, "==(_)", num_eqeq);
  PRIMITIVE(vm->numClass, "!=(_)", num_bangeq);

  wrenInterpret(vm, "", libSource);

  vm->stringClass = AS_CLASS(wrenFindVariable(vm, "String"));
  PRIMITIVE(vm->stringClass->obj.classObj, "fromCodePoint(_)", string_fromCodePoint);
  PRIMITIVE(vm->stringClass, "+(_)", string_plus);
  PRIMITIVE(vm->stringClass, "[_]", string_subscript);
  PRIMITIVE(vm->stringClass, "contains(_)", string_contains);
  PRIMITIVE(vm->stringClass, "count", string_count);
  PRIMITIVE(vm->stringClass, "endsWith(_)", string_endsWith);
  PRIMITIVE(vm->stringClass, "indexOf(_)", string_indexOf);
  PRIMITIVE(vm->stringClass, "iterate(_)", string_iterate);
  PRIMITIVE(vm->stringClass, "iteratorValue(_)", string_iteratorValue);
  PRIMITIVE(vm->stringClass, "startsWith(_)", string_startsWith);
  PRIMITIVE(vm->stringClass, "toString", string_toString);

  vm->listClass = AS_CLASS(wrenFindVariable(vm, "List"));
  PRIMITIVE(vm->listClass->obj.classObj, "<instantiate>", list_instantiate);
  PRIMITIVE(vm->listClass, "[_]", list_subscript);
  PRIMITIVE(vm->listClass, "[_]=(_)", list_subscriptSetter);
  PRIMITIVE(vm->listClass, "add(_)", list_add);
  PRIMITIVE(vm->listClass, "clear()", list_clear);
  PRIMITIVE(vm->listClass, "count", list_count);
  PRIMITIVE(vm->listClass, "insert(_,_)", list_insert);
  PRIMITIVE(vm->listClass, "iterate(_)", list_iterate);
  PRIMITIVE(vm->listClass, "iteratorValue(_)", list_iteratorValue);
  PRIMITIVE(vm->listClass, "removeAt(_)", list_removeAt);

  vm->mapClass = AS_CLASS(wrenFindVariable(vm, "Map"));
  PRIMITIVE(vm->mapClass->obj.classObj, "<instantiate>", map_instantiate);
  PRIMITIVE(vm->mapClass, "[_]", map_subscript);
  PRIMITIVE(vm->mapClass, "[_]=(_)", map_subscriptSetter);
  PRIMITIVE(vm->mapClass, "clear()", map_clear);
  PRIMITIVE(vm->mapClass, "containsKey(_)", map_containsKey);
  PRIMITIVE(vm->mapClass, "count", map_count);
  PRIMITIVE(vm->mapClass, "remove(_)", map_remove);
  PRIMITIVE(vm->mapClass, "iterate_(_)", map_iterate);
  PRIMITIVE(vm->mapClass, "keyIteratorValue_(_)", map_keyIteratorValue);
  PRIMITIVE(vm->mapClass, "valueIteratorValue_(_)", map_valueIteratorValue);

  vm->rangeClass = AS_CLASS(wrenFindVariable(vm, "Range"));
  PRIMITIVE(vm->rangeClass, "from", range_from);
  PRIMITIVE(vm->rangeClass, "to", range_to);
  PRIMITIVE(vm->rangeClass, "min", range_min);
  PRIMITIVE(vm->rangeClass, "max", range_max);
  PRIMITIVE(vm->rangeClass, "isInclusive", range_isInclusive);
  PRIMITIVE(vm->rangeClass, "iterate(_)", range_iterate);
  PRIMITIVE(vm->rangeClass, "iteratorValue(_)", range_iteratorValue);
  PRIMITIVE(vm->rangeClass, "toString", range_toString);

  // While bootstrapping the core types and running the core library, a number
  // of string objects have been created, many of which were instantiated
  // before stringClass was stored in the VM. Some of them *must* be created
  // first -- the ObjClass for string itself has a reference to the ObjString
  // for its name.
  //
  // These all currently have a NULL classObj pointer, so go back and assign
  // them now that the string class is known.
  for (Obj* obj = vm->first; obj != NULL; obj = obj->next)
  {
    if (obj->type == OBJ_STRING) obj->classObj = vm->stringClass;
  }
}
