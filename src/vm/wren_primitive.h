#ifndef wren_primitive_h
#define wren_primitive_h

#include "wren_vm.h"

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

// Validates that the given argument in [args] is a function. Returns true if
// it is. If not, reports an error and returns false.
bool validateFn(WrenVM* vm, Value* args, int index, const char* argName);

// Validates that the given argument in [args] is a Num. Returns true if it is.
// If not, reports an error and returns false.
bool validateNum(WrenVM* vm, Value* args, int index, const char* argName);

// Validates that [value] is an integer. Returns true if it is. If not, reports
// an error and returns false.
bool validateIntValue(WrenVM* vm, Value* args, double value,
                      const char* argName);

// Validates that the given argument in [args] is an integer. Returns true if
// it is. If not, reports an error and returns false.
bool validateInt(WrenVM* vm, Value* args, int index, const char* argName);

// Validates that [value] is an integer within `[0, count)`. Also allows
// negative indices which map backwards from the end. Returns the valid positive
// index value. If invalid, reports an error and returns `UINT32_MAX`.
uint32_t validateIndexValue(WrenVM* vm, Value* args, uint32_t count,
                            double value, const char* argName);

// Validates that [key] is a valid object for use as a map key. Returns true if
// it is. If not, reports an error and returns false.
bool validateKey(WrenVM* vm, Value* args, int index);

// Validates that the argument at [argIndex] is an integer within `[0, count)`.
// Also allows negative indices which map backwards from the end. Returns the
// valid positive index value. If invalid, reports an error and returns
// `UINT32_MAX`.
uint32_t validateIndex(WrenVM* vm, Value* args, uint32_t count, int arg,
                       const char* argName);

// Validates that the given argument in [args] is a String. Returns true if it
// is. If not, reports an error and returns false.
bool validateString(WrenVM* vm, Value* args, int index, const char* argName);

// Given a [range] and the [length] of the object being operated on, determines
// the series of elements that should be chosen from the underlying object.
// Handles ranges that count backwards from the end as well as negative ranges.
//
// Returns the index from which the range should start or `UINT32_MAX` if the
// range is invalid. After calling, [length] will be updated with the number of
// elements in the resulting sequence. [step] will be direction that the range
// is going: `1` if the range is increasing from the start index or `-1` if the
// range is decreasing.
uint32_t calculateRange(WrenVM* vm, Value* args, ObjRange* range,
                        uint32_t* length, int* step);

#endif
