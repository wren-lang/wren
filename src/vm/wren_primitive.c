#include "wren_primitive.h"

#include <math.h>

// Validates that [value] is an integer within `[0, count)`. Also allows
// negative indices which map backwards from the end. Returns the valid positive
// index value. If invalid, reports an error and returns `UINT32_MAX`.
static uint32_t validateIndexValue(WrenFiber* fiber, uint32_t count, double value,
                                   const char* argName)
{
  if (!validateIntValue(fiber, value, argName)) return UINT32_MAX;
  
  // Negative indices count from the end.
  if (value < 0) value = count + value;
  
  // Check bounds.
  if (value >= 0 && value < count) return (uint32_t)value;
  
  fiber->error = wrenStringFormat(fiber->vm, "$ out of bounds.", argName);
  return UINT32_MAX;
}

bool validateFn(WrenFiber* fiber, Value arg, const char* argName)
{
  if (IS_CLOSURE(arg)) return true;

  RETURN_ERROR_FMT("$ must be a function.", argName);
}

bool validateNum(WrenFiber* fiber, Value arg, const char* argName)
{
  if (IS_NUM(arg)) return true;
  RETURN_ERROR_FMT("$ must be a number.", argName);
}

bool validateIntValue(WrenFiber* fiber, double value, const char* argName)
{
  if (trunc(value) == value) return true;
  RETURN_ERROR_FMT("$ must be an integer.", argName);
}

bool validateInt(WrenFiber* fiber, Value arg, const char* argName)
{
  // Make sure it's a number first.
  if (!validateNum(fiber, arg, argName)) return false;
  return validateIntValue(fiber, AS_NUM(arg), argName);
}

bool validateKey(WrenFiber* fiber, Value arg)
{
  if (IS_BOOL(arg) || IS_CLASS(arg) || IS_NULL(arg) ||
      IS_NUM(arg) || IS_RANGE(arg) || IS_STRING(arg))
  {
    return true;
  }

  RETURN_ERROR("Key must be a value type.");
}

uint32_t validateIndex(WrenFiber* fiber, Value arg, uint32_t count,
                       const char* argName)
{
  if (!validateNum(fiber, arg, argName)) return UINT32_MAX;
  return validateIndexValue(fiber, count, AS_NUM(arg), argName);
}

bool validateString(WrenFiber* fiber, Value arg, const char* argName)
{
  if (IS_STRING(arg)) return true;
  RETURN_ERROR_FMT("$ must be a string.", argName);
}

uint32_t calculateRange(WrenFiber* fiber, ObjRange* range, uint32_t* length,
                        int* step)
{
  *step = 0;

  // Edge case: an empty range is allowed at the end of a sequence. This way,
  // list[0..-1] and list[0...list.count] can be used to copy a list even when
  // empty.
  if (range->from == *length &&
      range->to == (range->isInclusive ? -1.0 : (double)*length))
  {
    *length = 0;
    return 0;
  }

  uint32_t from = validateIndexValue(fiber, *length, range->from, "Range start");
  if (from == UINT32_MAX) return UINT32_MAX;

  // Bounds check the end manually to handle exclusive ranges.
  double value = range->to;
  if (!validateIntValue(fiber, value, "Range end")) return UINT32_MAX;

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
    fiber->error = CONST_STRING(fiber->vm, "Range end out of bounds.");
    return UINT32_MAX;
  }

  uint32_t to = (uint32_t)value;
  *length = abs((int)(from - to)) + 1;
  *step = from < to ? 1 : -1;
  return from;
}
