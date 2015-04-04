#include "wren_primitive.h"

#include <math.h>

bool validateFn(WrenVM* vm, Value* args, int index, const char* argName)
{
  if (IS_FN(args[index]) || IS_CLOSURE(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a function.", argName);
  return false;
}

bool validateNum(WrenVM* vm, Value* args, int index, const char* argName)
{
  if (IS_NUM(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a number.", argName);
  return false;
}

bool validateIntValue(WrenVM* vm, Value* args, double value,
                      const char* argName)
{
  if (trunc(value) == value) return true;

  args[0] = wrenStringFormat(vm, "$ must be an integer.", argName);
  return false;
}

bool validateInt(WrenVM* vm, Value* args, int index, const char* argName)
{
  // Make sure it's a number first.
  if (!validateNum(vm, args, index, argName)) return false;

  return validateIntValue(vm, args, AS_NUM(args[index]), argName);
}

uint32_t validateIndexValue(WrenVM* vm, Value* args, uint32_t count,
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

bool validateKey(WrenVM* vm, Value* args, int index)
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

uint32_t validateIndex(WrenVM* vm, Value* args, uint32_t count, int arg,
                       const char* argName)
{
  if (!validateNum(vm, args, arg, argName)) return UINT32_MAX;

  return validateIndexValue(vm, args, count, AS_NUM(args[arg]), argName);
}

bool validateString(WrenVM* vm, Value* args, int index, const char* argName)
{
  if (IS_STRING(args[index])) return true;

  args[0] = wrenStringFormat(vm, "$ must be a string.", argName);
  return false;
}

uint32_t calculateRange(WrenVM* vm, Value* args, ObjRange* range,
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
  *length = abs((int)(from - to)) + 1;
  *step = from < to ? 1 : -1;
  return from;
}
