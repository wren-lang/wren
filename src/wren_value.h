#ifndef wren_value_h
#define wren_value_h

#include <stdint.h>

#include "wren_common.h"
#include "wren.h"

// This defines the built-in types and their core representations in memory.
// Since Wren is dynamically typed, any variable can hold a value of any type,
// and the type can change at runtime. Implementing this efficiently is
// critical for performance.
//
// The main type exposed by this is [Value]. A C variable of that type is a
// storage location that can hold any Wren value. The stack, global variables,
// and instance fields are all implemented in C as variables of type Value.
//
// The built-in types for booleans, numbers, and null are unboxed: their value
// is stored directly in the Value, and copying a Value copies the value. Other
// types--classes, instances of classes, functions, lists, and strings--are all
// reference types. They are stored on the heap and the Value just stores a
// pointer to it. Copying the Value copies a reference to the same object. The
// Wren implementation calls these "Obj", or objects, though to a user, all
// values are objects.
//
// There are two supported Value representations. The main one uses a technique
// called "NaN tagging" (explained in detail below) to store a number, any of
// the value types, or a pointer all inside a single double-precision floating
// point value. A larger, slower, Value type that uses a struct to store these
// is also supported, and is useful for debugging the VM.
//
// The representation is controlled by the `NAN_TAGGING` define. If that's
// defined, Nan tagging is used.

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
  OBJ_CLOSURE,
  OBJ_FN,
  OBJ_INSTANCE,
  OBJ_LIST,
  OBJ_STRING,
  OBJ_UPVALUE
} ObjType;

typedef enum
{
  // The object has been marked during the mark phase of GC.
  FLAG_MARKED = 0x01,
} ObjFlags;

typedef struct sObj
{
  unsigned int type  : 3; // ObjType.
  unsigned int flags : 1; // ObjFlags.

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

// A first-class function object. A raw ObjFn can be used and invoked directly
// if it has no upvalues (i.e. [numUpvalues] is zero). If it does use upvalues,
// it must be wrapped in an [ObjClosure] first. The compiler is responsible for
// emitting code to ensure that that happens.
typedef struct
{
  Obj obj;
  int numConstants;
  int numUpvalues;
  unsigned char* bytecode;

  // TODO(bob): Flexible array?
  Value* constants;
} ObjFn;

// The dynamically allocated data structure for a variable that has been used
// by a closure. Whenever a function accesses a variable declared in an
// enclosing function, it will get to it through this.
//
// An upvalue can be either "closed" or "open". An open upvalue points directly
// to a [Value] that is still stored on the fiber's stack because the local
// variable is still in scope in the function where it's declared.
//
// When that local variable goes out of scope, the upvalue pointing to it will
// be closed. When that happens, the value gets copied off the stack into the
// upvalue itself. That way, it can have a longer lifetime than the stack
// variable.
typedef struct sUpvalue
{
  // The object header. Note that upvalues have this because they are garbage
  // collected, but they are not first class Wren objects.
  Obj obj;

  // Pointer to the variable this upvalue is referencing.
  Value* value;

  // If the upvalue is closed (i.e. the local variable it was pointing too has
  // been popped off the stack) then the closed-over value will be hoisted out
  // of the stack into here. [value] will then be changed to point to this.
  Value closed;

  // Open upvalues are stored in a linked list by the fiber. This points to the
  // next upvalue in that list.
  struct sUpvalue* next;
} Upvalue;

// An instance of a first-class function and the environment it has closed over.
// Unlike [ObjFn], this has captured the upvalues that the function accesses.
typedef struct
{
  Obj obj;

  // The function that this closure is an instance of.
  ObjFn* fn;

  // TODO(bob): Make a flexible array.
  // The upvalues this function has closed over.
  Upvalue** upvalues;
} ObjClosure;

typedef enum
{
  // A primitive method implemented in C that immediately returns a value.
  METHOD_PRIMITIVE,

  // A built-in method that modifies the fiber directly.
  METHOD_FIBER,

  // A normal user-defined method.
  METHOD_BLOCK,

  // No method for the given symbol.
  METHOD_NONE
} MethodType;

typedef struct
{
  MethodType type;

  // The method function itself. The [type] determines which field of the union
  // is used.
  union
  {
    Primitive primitive;
    FiberPrimitive fiberPrimitive;

    // May be a [ObjFn] or [ObjClosure].
    Value fn;
  };
} Method;

typedef struct sObjClass
{
  Obj obj;
  struct sObjClass* metaclass;
  struct sObjClass* superclass;

  // The number of fields needed for an instance of this class, including all
  // of its superclass fields.
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

// Value -> ObjClosure*.
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))

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
#define IS_BOOL(value) (wrenIsBool(value))

// Returns non-zero if [value] is a closure.
#define IS_CLOSURE(value) (wrenIsClosure(value))

// Returns non-zero if [value] is a function object.
#define IS_FN(value) (wrenIsFn(value))

// Returns non-zero if [value] is an instance.
#define IS_INSTANCE(value) (wrenIsInstance(value))

// Returns non-zero if [value] is a string object.
#define IS_STRING(value) (wrenIsString(value))


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
#define OBJ_VAL(obj) (wrenObjectToValue((Obj*)(obj)))

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
#define OBJ_VAL(obj) (wrenObjectToValue((Obj*)(obj)))

// double -> Value.
#define NUM_VAL(n) ((Value){ VAL_NUM, n, NULL })

// Singleton values.
#define FALSE_VAL ((Value){ VAL_FALSE, 0.0, NULL })
#define NULL_VAL ((Value){ VAL_NULL, 0.0, NULL })
#define TRUE_VAL ((Value){ VAL_TRUE, 0.0, NULL })

#endif

// Creates a new class object.
ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields);

// Creates a new closure object that invokes [fn]. Allocates room for its
// upvalues, but assumes outside code will populate it.
ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn);

// Creates a new function object. Assumes the compiler will fill it in with
// bytecode, constants, etc.
ObjFn* wrenNewFunction(WrenVM* vm);

// Creates a new instance of the given [classObj].
Value wrenNewInstance(WrenVM* vm, ObjClass* classObj);

// Creates a new list with [numElements] elements (which are left
// uninitialized.)
ObjList* wrenNewList(WrenVM* vm, int numElements);

// Creates a new string object and copies [text] into it.
Value wrenNewString(WrenVM* vm, const char* text, size_t length);

// Creates a new open upvalue pointing to [value] on the stack.
Upvalue* wrenNewUpvalue(WrenVM* vm, Value* value);

// Returns the class of [value].
ObjClass* wrenGetClass(WrenVM* vm, Value value);

// Returns non-zero if [a] and [b] are strictly equal using built-in equality
// semantics. This is identity for object values, and value equality for others.
int wrenValuesEqual(Value a, Value b);

void wrenPrintValue(Value value);

int wrenIsBool(Value value);
int wrenIsClosure(Value value);
int wrenIsFn(Value value);
int wrenIsInstance(Value value);
int wrenIsString(Value value);
Value wrenObjectToValue(Obj* obj);

#endif
