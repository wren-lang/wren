#ifndef wren_value_h
#define wren_value_h

#include <stdint.h>

#include "common.h"
#include "wren.h"

// TODO(bob): This should be in VM. (Or, really, we shouldn't hardcode this at
// all and have growable symbol tables.)
#define MAX_SYMBOLS 256

typedef enum
{
  VAL_FALSE,
  VAL_NULL,
  VAL_NUM,
  VAL_TRUE,
  VAL_OBJ
} ValueType;

typedef enum {
  OBJ_CLASS,
  OBJ_FN,
  OBJ_INSTANCE,
  OBJ_LIST,
  OBJ_STRING
} ObjType;

typedef enum
{
  // The object has been marked during the mark phase of GC.
  FLAG_MARKED = 0x01,
} ObjFlags;

typedef struct sObj
{
  ObjType type   : 3;
  ObjFlags flags : 1;

  // The next object in the linked list of all currently allocated objects.
  struct sObj* next;
} Obj;

#ifdef NAN_TAGGING

typedef union
{
  double num;
  uint64_t bits;
} Value;

#else

typedef struct
{
  ValueType type;
  double num;
  Obj* obj;
} Value;

#endif

typedef struct sFiber Fiber;

typedef Value (*Primitive)(WrenVM* vm, Value* args);
typedef void (*FiberPrimitive)(WrenVM* vm, Fiber* fiber, Value* args);

typedef struct
{
  Obj obj;
  unsigned char* bytecode;
  Value* constants;
  int numConstants;
} ObjFn;

typedef enum
{
  // No method for the given symbol.
  METHOD_NONE,

  // A primitive method implemented in C that immediately returns a value.
  METHOD_PRIMITIVE,

  // A built-in method that modifies the fiber directly.
  METHOD_FIBER,

  // A normal user-defined method.
  METHOD_BLOCK,

  // A constructor. This will be defined on the metaclass. If [fn] is non-NULL,
  // then it's a user-defined constructor and [fn] is the initialization code.
  // Otherwise, it's a default constructor.
  METHOD_CTOR
} MethodType;

typedef struct
{
  MethodType type;
  union
  {
    Primitive primitive;
    FiberPrimitive fiberPrimitive;
    ObjFn* fn;
  };
} Method;

typedef struct sObjClass
{
  Obj obj;
  struct sObjClass* metaclass;
  struct sObjClass* superclass;
  int numFields;
  // TODO(bob): Hack. Probably don't want to use this much space.
  Method methods[MAX_SYMBOLS];
} ObjClass;

typedef struct
{
  Obj obj;
  ObjClass* classObj;
  Value fields[];
} ObjInstance;

typedef struct
{
  Obj obj;

  // The number of elements allocated.
  int capacity;

  // The number of items in the list.
  int count;

  // Pointer to a contiguous array of [capacity] elements.
  Value* elements;
} ObjList;

typedef struct
{
  Obj obj;
  char* value;
} ObjString;


// Value -> ObjClass*.
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))

// Value -> ObjFn*.
#define AS_FN(value) ((ObjFn*)AS_OBJ(value))

// Value -> ObjInstance*.
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

// Value -> ObjList*.
#define AS_LIST(value) ((ObjList*)AS_OBJ(value))

// Value -> double.
#define AS_NUM(v) ((v).num)

// Value -> ObjString*.
#define AS_STRING(v) ((ObjString*)AS_OBJ(v))

// Value -> const char*.
#define AS_CSTRING(v) (AS_STRING(v)->value)

// Convert [boolean], an int, to a boolean [Value].
#define BOOL_VAL(boolean) (boolean ? TRUE_VAL : FALSE_VAL)

// Returns non-zero if [value] is a bool.
#define IS_BOOL(value) (valueIsBool(value))

// Returns non-zero if [value] is a function object.
#define IS_FN(value) (valueIsFn(value))

// Returns non-zero if [value] is an instance.
#define IS_INSTANCE(value) (valueIsInstance(value))

// Returns non-zero if [value] is a string object.
#define IS_STRING(value) (valueIsString(value))


#ifdef NAN_TAGGING

// An IEEE 754 double-precision float is a 64-bit value with bits laid out like:
//
// 1 Sign bit
// | 11 Exponent bits
// | |           52 Mantissa (i.e. fraction) bits
// | |           |
// S(Exponent--)(Mantissa-----------------------------------------)
//
// The details of how these are used to represent numbers aren't really
// relevant here as long we don't interfere with them. The important bit is NaN.
//
// An IEEE double can represent a few magical values like NaN ("not a number"),
// Infinity, and -Infinity. A NaN is any value where all exponent bits are set:
//
// v--NaN bits
// -111111111111---------------------------------------------------
//
// Here, "-" means "doesn't matter". Any bit sequence that matches the above is
// a NaN. With all of those "-", it obvious there are a *lot* of different
// bit patterns that all mean the same thing. NaN tagging takes advantage of
// this. We'll use those available bit patterns to represent things other than
// numbers without giving up any valid numeric values.
//
// NaN values come in two flavors: "signalling" and "quiet". The former are
// intended to halt execution, while the latter just flow through arithmetic
// operations silently. We want the latter. Quiet NaNs are indicated by setting
// the highest mantissa bit:
//
// v--Mantissa bit
// -[NaN       ]1--------------------------------------------------
//
// If all of the NaN bits are set, it's not a number. Otherwise, it is.
// That leaves all of the remaining bits as available for us to play with. We
// stuff a few different kinds of things here: special singleton values like
// "true", "false", and "null", and pointers to objects allocated on the heap.
// We'll use the sign bit to distinguish singleton values from pointers. If it's
// set, it's a pointer.
//
// v--Pointer or singleton?
// S[NaN       ]1-----0--------------------------------------------
//
// For singleton values, we just to enumerate the different values. We'll use
// the low three bits of the mantissa for that, and only need a couple:
//
// 3 Type bits--v
// 0[NaN       ]1------0----------------------------------------[T]
//
// For pointers, we are left with 48 bits of mantissa to store an address.
// That's more than enough room for a 32-bit address. Even 64-bit machines
// only actually use 48 bits for addresses, so we've got plenty. We just stuff
// the address right into the mantissa.
//
// Ta-da, double precision numbers, pointers, and a bunch of singleton values,
// all stuffed into a single 64-bit sequence. Even better, we don't have to
// do any masking or work to extract number values: they are unmodified. This
// means math on numbers is fast.

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

// If the NaN bits are set, it's not a number.
#define IS_NUM(value) (((value).bits & QNAN) != QNAN)

// Singleton values are NaN with the sign bit cleared. (This includes the
// normal value of the actual NaN value used in numeric arithmetic.)
#define IS_SINGLETON(value) (((value).bits & (QNAN | SIGN_BIT)) == QNAN)

// An object pointer is a NaN with a set sign bit.
#define IS_OBJ(value) (((value).bits & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define IS_FALSE(value) ((value).bits == FALSE_VAL.bits)
#define IS_NULL(value) ((value).bits == (QNAN | TAG_NULL))

// Masks out the tag bits used to identify the singleton value.
#define MASK_TAG (7)

// Tag values for the different singleton values.
#define TAG_NAN     (0)
#define TAG_NULL    (1)
#define TAG_FALSE   (2)
#define TAG_TRUE    (3)
#define TAG_UNUSED1 (4)
#define TAG_UNUSED2 (5)
#define TAG_UNUSED3 (6)
#define TAG_UNUSED4 (7)

// double -> Value.
#define NUM_VAL(n) ((Value)(double)(n))

// Value -> 0 or 1.
#define AS_BOOL(value) ((value).bits == TRUE_VAL.bits)

// Value -> Obj*.
#define AS_OBJ(value) ((Obj*)((value).bits & ~(SIGN_BIT | QNAN)))

// Singleton values.
#define NULL_VAL   ((Value)(uint64_t)(QNAN | TAG_NULL))
#define FALSE_VAL  ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL   ((Value)(uint64_t)(QNAN | TAG_TRUE))

// Gets the singleton type tag for a Value (which must be a singleton).
#define GET_TAG(value) ((int)((value).bits & MASK_TAG))

// Converts a pointer to an Obj to a Value.
#define OBJ_VAL(obj) (objectToValue((Obj*)(obj)))

#else

// Value -> 0 or 1.
#define AS_BOOL(value) ((value).type == VAL_TRUE)

// Value -> Obj*.
#define AS_OBJ(v) ((v).obj)

// Determines if [value] is a garbage-collected object or not.
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define IS_FALSE(value) ((value).type == VAL_FALSE)
#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUM(value) ((value).type == VAL_NUM)

// Convert [obj], an `Obj*`, to a [Value].
#define OBJ_VAL(obj) (objectToValue((Obj*)(obj)))

// double -> Value.
#define NUM_VAL(n) ((Value){ VAL_NUM, n, NULL })

// Singleton values.
#define FALSE_VAL ((Value){ VAL_FALSE, 0.0, NULL })
#define NULL_VAL ((Value){ VAL_NULL, 0.0, NULL })
#define TRUE_VAL ((Value){ VAL_TRUE, 0.0, NULL })

#endif

// Returns non-zero if [a] and [b] are strictly equal using built-in equality
// semantics. This is identity for object values, and value equality for others.
int wrenValuesEqual(Value a, Value b);

// Returns the class of [value].
ObjClass* wrenGetClass(WrenVM* vm, Value value);

void wrenPrintValue(Value value);

int valueIsBool(Value value);
int valueIsFn(Value value);
int valueIsInstance(Value value);
int valueIsString(Value value);
Value objectToValue(Obj* obj);

#endif
