#ifndef wren_value_h
#define wren_value_h

#include <stdbool.h>
#include <stdint.h>

#include "wren_common.h"
#include "wren_utils.h"

// This defines the built-in types and their core representations in memory.
// Since Wren is dynamically typed, any variable can hold a value of any type,
// and the type can change at runtime. Implementing this efficiently is
// critical for performance.
//
// The main type exposed by this is [Value]. A C variable of that type is a
// storage location that can hold any Wren value. The stack, module variables,
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
// There is also a special singleton value "undefined". It is used internally
// but never appears as a real value to a user. It has two uses:
//
// - It is used to identify module variables that have been implicitly declared
//   by use in a forward reference but not yet explicitly declared. These only
//   exist during compilation and do not appear at runtime.
//
// - It is used to represent unused map entries in an ObjMap.
//
// There are two supported Value representations. The main one uses a technique
// called "NaN tagging" (explained in detail below) to store a number, any of
// the value types, or a pointer all inside a single double-precision floating
// point value. A larger, slower, Value type that uses a struct to store these
// is also supported, and is useful for debugging the VM.
//
// The representation is controlled by the `NAN_TAGGING` define. If that's
// defined, Nan tagging is used.

// TODO: Make these externally controllable.
#define STACK_SIZE 1024
#define MAX_CALL_FRAMES 256

// TODO: Can we eliminate this and use the classObj pointers to tell an object's
// type instead?
// Identifies which specific type a heap-allocated object is.
typedef enum {
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FIBER,
  OBJ_FN,
  OBJ_INSTANCE,
  OBJ_LIST,
  OBJ_MAP,
  OBJ_MODULE,
  OBJ_RANGE,
  OBJ_STRING,
  OBJ_UPVALUE
} ObjType;

typedef struct sObjClass ObjClass;

// Base struct for all heap-allocated objects.
typedef struct sObj
{
  ObjType type;
  bool marked;

  // The object's class.
  ObjClass* classObj;

  // The next object in the linked list of all currently allocated objects.
  struct sObj* next;
} Obj;

#if WREN_NAN_TAGGING

typedef uint64_t Value;

#else

typedef enum
{
  VAL_FALSE,
  VAL_NULL,
  VAL_NUM,
  VAL_TRUE,
  VAL_UNDEFINED,
  VAL_OBJ
} ValueType;

typedef struct
{
  ValueType type;
  double num;
  Obj* obj;
} Value;

#endif

DECLARE_BUFFER(Value, Value);

typedef struct
{
  Obj obj;
  // Does not include the null terminator.
  int length;
  char value[FLEXIBLE_ARRAY];
} ObjString;

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

typedef struct
{
  // Pointer to the current (really next-to-be-executed) instruction in the
  // function's bytecode.
  uint8_t* ip;

  // The function or closure being executed.
  Obj* fn;

  // Pointer to the first stack slot used by this call frame. This will contain
  // the receiver, followed by the function's parameters, then local variables
  // and temporaries.
  Value* stackStart;
} CallFrame;

typedef struct sObjFiber
{
  Obj obj;
  Value stack[STACK_SIZE];
  Value* stackTop;

  CallFrame frames[MAX_CALL_FRAMES];
  int numFrames;

  // Pointer to the first node in the linked list of open upvalues that are
  // pointing to values still on the stack. The head of the list will be the
  // upvalue closest to the top of the stack, and then the list works downwards.
  Upvalue* openUpvalues;

  // The fiber that ran this one. If this fiber is yielded, control will resume
  // to this one. May be `NULL`.
  struct sObjFiber* caller;

  // If the fiber failed because of a runtime error, this will contain the
  // error message. Otherwise, it will be NULL.
  ObjString* error;

  // This will be true if the caller that called this fiber did so using "try".
  // In that case, if this fiber fails with an error, the error will be given
  // to the caller.
  bool callerIsTrying;
} ObjFiber;

typedef enum
{
  // A normal value has been returned.
  PRIM_VALUE,

  // A runtime error occurred.
  PRIM_ERROR,

  // A new callframe has been pushed.
  PRIM_CALL,

  // A fiber is being switched to.
  PRIM_RUN_FIBER

} PrimitiveResult;

typedef PrimitiveResult (*Primitive)(WrenVM* vm, ObjFiber* fiber, Value* args);

// TODO: See if it's actually a perf improvement to have this in a separate
// struct instead of in ObjFn.
// Stores debugging information for a function used for things like stack
// traces.
typedef struct
{
  // The name of the function. Heap allocated and owned by the ObjFn.
  char* name;

  // The name of the source file where this function was defined. An [ObjString]
  // because this will be shared among all functions defined in the same file.
  ObjString* sourcePath;

  // An array of line numbers. There is one element in this array for each
  // bytecode in the function's bytecode array. The value of that element is
  // the line in the source code that generated that instruction.
  int* sourceLines;
} FnDebug;

// A loaded module and the top-level variables it defines.
//
// While this is an Obj and is managed by the GC, it never appears as a
// first-class object in Wren.
typedef struct
{
  Obj obj;

  // The currently defined top-level variables.
  ValueBuffer variables;

  // Symbol table for the names of all module variables. Indexes here directly
  // correspond to entries in [variables].
  SymbolTable variableNames;
} ObjModule;

// A first-class function object. A raw ObjFn can be used and invoked directly
// if it has no upvalues (i.e. [numUpvalues] is zero). If it does use upvalues,
// it must be wrapped in an [ObjClosure] first. The compiler is responsible for
// emitting code to ensure that that happens.
typedef struct
{
  Obj obj;
  // TODO: Make one of these a flexible array? I tried each and it didn't seem
  // to help perf, but it bears more investigation.
  Value* constants;
  uint8_t* bytecode;

  // The module where this function was defined.
  ObjModule* module;

  int numUpvalues;
  int numConstants;

  // TODO: Move to FnDebug?
  int bytecodeLength;

  // The number of parameters this function expects. Used to ensure that .call
  // handles a mismatch between number of parameters and arguments. This will
  // only be set for fns, and not ObjFns that represent methods or scripts.
  int arity;
  FnDebug* debug;
} ObjFn;

// An instance of a first-class function and the environment it has closed over.
// Unlike [ObjFn], this has captured the upvalues that the function accesses.
typedef struct
{
  Obj obj;

  // The function that this closure is an instance of.
  ObjFn* fn;

  // The upvalues this function has closed over.
  Upvalue* upvalues[FLEXIBLE_ARRAY];
} ObjClosure;

typedef enum
{
  // A primitive method implemented in C in the VM. Unlike foreign methods,
  // this can directly manipulate the fiber's stack.
  METHOD_PRIMITIVE,

  // A externally-defined C method.
  METHOD_FOREIGN,

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
    WrenForeignMethodFn foreign;

    // May be a [ObjFn] or [ObjClosure].
    Obj* obj;
  } fn;
} Method;

DECLARE_BUFFER(Method, Method);

struct sObjClass
{
  Obj obj;
  ObjClass* superclass;

  // The number of fields needed for an instance of this class, including all
  // of its superclass fields.
  int numFields;

  // The table of methods that are defined in or inherited by this class.
  // Methods are called by symbol, and the symbol directly maps to an index in
  // this table. This makes method calls fast at the expense of empty cells in
  // the list for methods the class doesn't support.
  //
  // You can think of it as a hash table that never has collisions but has a
  // really low load factor. Since methods are pretty small (just a type and a
  // pointer), this should be a worthwhile trade-off.
  MethodBuffer methods;

  // The name of the class.
  ObjString* name;
};

typedef struct
{
  Obj obj;
  Value fields[FLEXIBLE_ARRAY];
} ObjInstance;

typedef struct
{
  Obj obj;

  // TODO: Make these uint32_t to match ObjMap, or vice versa.
  
  // The number of elements allocated.
  int capacity;

  // The number of items in the list.
  int count;

  // Pointer to a contiguous array of [capacity] elements.
  Value* elements;
} ObjList;

typedef struct
{
  // The entry's key, or UNDEFINED if the entry is not in use.
  Value key;

  // The value associated with the key.
  Value value;
} MapEntry;

// A hash table mapping keys to values.
typedef struct
{
  Obj obj;

  // The number of entries allocated.
  uint32_t capacity;

  // The number of entries in the map.
  uint32_t count;

  // Pointer to a contiguous array of [capacity] entries.
  MapEntry* entries;
} ObjMap;

typedef struct
{
  Obj obj;

  // The beginning of the range.
  double from;

  // The end of the range. May be greater or less than [from].
  double to;

  // True if [to] is included in the range.
  bool isInclusive;
} ObjRange;


// Value -> ObjClass*.
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))

// Value -> ObjClosure*.
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))

// Value -> ObjFiber*.
#define AS_FIBER(v) ((ObjFiber*)AS_OBJ(v))

// Value -> ObjFn*.
#define AS_FN(value) ((ObjFn*)AS_OBJ(value))

// Value -> ObjInstance*.
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))

// Value -> ObjList*.
#define AS_LIST(value) ((ObjList*)AS_OBJ(value))

// Value -> ObjMap*.
#define AS_MAP(value) ((ObjMap*)AS_OBJ(value))

// Value -> ObjModule*.
#define AS_MODULE(value) ((ObjModule*)AS_OBJ(value))

// Value -> double.
#define AS_NUM(value) (wrenValueToNum(value))

// Value -> ObjRange*.
#define AS_RANGE(v) ((ObjRange*)AS_OBJ(v))

// Value -> ObjString*.
#define AS_STRING(v) ((ObjString*)AS_OBJ(v))

// Value -> const char*.
#define AS_CSTRING(v) (AS_STRING(v)->value)

// Convert [boolean] to a boolean [Value].
#define BOOL_VAL(boolean) (boolean ? TRUE_VAL : FALSE_VAL)

// double -> Value.
#define NUM_VAL(num) (wrenNumToValue(num))

// Convert [obj], an `Obj*`, to a [Value].
#define OBJ_VAL(obj) (wrenObjectToValue((Obj*)(obj)))

// Returns true if [value] is a bool.
#define IS_BOOL(value) (wrenIsBool(value))

// Returns true if [value] is a class.
#define IS_CLASS(value) (wrenIsObjType(value, OBJ_CLASS))

// Returns true if [value] is a closure.
#define IS_CLOSURE(value) (wrenIsObjType(value, OBJ_CLOSURE))

// Returns true if [value] is a function object.
#define IS_FN(value) (wrenIsObjType(value, OBJ_FN))

// Returns true if [value] is an instance.
#define IS_INSTANCE(value) (wrenIsObjType(value, OBJ_INSTANCE))

// Returns true if [value] is a range object.
#define IS_RANGE(value) (wrenIsObjType(value, OBJ_RANGE))

// Returns true if [value] is a string object.
#define IS_STRING(value) (wrenIsObjType(value, OBJ_STRING))


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
// We'll use the sign bit to distinguish singleton values from pointers. If
// it's set, it's a pointer.
//
// v--Pointer or singleton?
// S[NaN       ]1--------------------------------------------------
//
// For singleton values, we just enumerate the different values. We'll use the
// low three bits of the mantissa for that, and only need a couple:
//
// 3 Type bits--v
// 0[NaN       ]1-----------------------------------------------[T]
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
#if WREN_NAN_TAGGING

// A mask that selects the sign bit.
#define SIGN_BIT ((uint64_t)1 << 63)

// The bits that must be set to indicate a quiet NaN.
#define QNAN ((uint64_t)0x7ffc000000000000)

// If the NaN bits are set, it's not a number.
#define IS_NUM(value) (((value) & QNAN) != QNAN)

// An object pointer is a NaN with a set sign bit.
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define IS_FALSE(value)     ((value) == FALSE_VAL)
#define IS_NULL(value)      ((value) == NULL_VAL)
#define IS_UNDEFINED(value) ((value) == UNDEFINED_VAL)

// Masks out the tag bits used to identify the singleton value.
#define MASK_TAG (7)

// Tag values for the different singleton values.
#define TAG_NAN       (0)
#define TAG_NULL      (1)
#define TAG_FALSE     (2)
#define TAG_TRUE      (3)
#define TAG_UNDEFINED (4)
#define TAG_UNUSED2   (5)
#define TAG_UNUSED3   (6)
#define TAG_UNUSED4   (7)

// Value -> 0 or 1.
#define AS_BOOL(value) ((value) == TRUE_VAL)

// Value -> Obj*.
#define AS_OBJ(value) ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

// Singleton values.
#define NULL_VAL      ((Value)(uint64_t)(QNAN | TAG_NULL))
#define FALSE_VAL     ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL      ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define UNDEFINED_VAL ((Value)(uint64_t)(QNAN | TAG_UNDEFINED))

// Gets the singleton type tag for a Value (which must be a singleton).
#define GET_TAG(value) ((int)((value) & MASK_TAG))

#else

// Value -> 0 or 1.
#define AS_BOOL(value) ((value).type == VAL_TRUE)

// Value -> Obj*.
#define AS_OBJ(v) ((v).obj)

// Determines if [value] is a garbage-collected object or not.
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define IS_FALSE(value)     ((value).type == VAL_FALSE)
#define IS_NULL(value)      ((value).type == VAL_NULL)
#define IS_NUM(value)       ((value).type == VAL_NUM)
#define IS_UNDEFINED(value) ((value).type == VAL_UNDEFINED)

// Singleton values.
#define FALSE_VAL     ((Value){ VAL_FALSE, 0.0, NULL })
#define NULL_VAL      ((Value){ VAL_NULL, 0.0, NULL })
#define TRUE_VAL      ((Value){ VAL_TRUE, 0.0, NULL })
#define UNDEFINED_VAL ((Value){ VAL_UNDEFINED, 0.0, NULL })

#endif

// A union to let us reinterpret a double as raw bits and back.
typedef union
{
  uint64_t bits64;
  uint32_t bits32[2];
  double num;
} DoubleBits;

// Creates a new "raw" class. It has no metaclass or superclass whatsoever.
// This is only used for bootstrapping the initial Object and Class classes,
// which are a little special.
ObjClass* wrenNewSingleClass(WrenVM* vm, int numFields, ObjString* name);

// Makes [superclass] the superclass of [subclass], and causes subclass to
// inherit its methods. This should be called before any methods are defined
// on subclass.
void wrenBindSuperclass(WrenVM* vm, ObjClass* subclass, ObjClass* superclass);

// Creates a new class object as well as its associated metaclass.
ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields,
                       ObjString* name);

void wrenBindMethod(WrenVM* vm, ObjClass* classObj, int symbol, Method method);

// Creates a new closure object that invokes [fn]. Allocates room for its
// upvalues, but assumes outside code will populate it.
ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn);

// Creates a new fiber object that will invoke [fn], which can be a function or
// closure.
ObjFiber* wrenNewFiber(WrenVM* vm, Obj* fn);

// TODO: The argument list here is getting a bit gratuitous.
// Creates a new function object with the given code and constants. The new
// function will take over ownership of [bytecode] and [sourceLines]. It will
// copy [constants] into its own array.
ObjFn* wrenNewFunction(WrenVM* vm, ObjModule* module,
                       Value* constants, int numConstants,
                       int numUpvalues, int arity,
                       uint8_t* bytecode, int bytecodeLength,
                       ObjString* debugSourcePath,
                       const char* debugName, int debugNameLength,
                       int* sourceLines);

// Creates a new instance of the given [classObj].
Value wrenNewInstance(WrenVM* vm, ObjClass* classObj);

// Creates a new list with [numElements] elements (which are left
// uninitialized.)
ObjList* wrenNewList(WrenVM* vm, int numElements);

// Adds [value] to [list], reallocating and growing its storage if needed.
void wrenListAdd(WrenVM* vm, ObjList* list, Value value);

// Inserts [value] in [list] at [index], shifting down the other elements.
void wrenListInsert(WrenVM* vm, ObjList* list, Value value, int index);

// Removes and returns the item at [index] from [list].
Value wrenListRemoveAt(WrenVM* vm, ObjList* list, int index);

// Creates a new empty map.
ObjMap* wrenNewMap(WrenVM* vm);

// Looks up [key] in [map]. If found, returns the index of its entry. Otherwise,
// returns `UINT32_MAX`.
uint32_t wrenMapFind(ObjMap* map, Value key);

// Associates [key] with [value] in [map].
void wrenMapSet(WrenVM* vm, ObjMap* map, Value key, Value value);

void wrenMapClear(WrenVM* vm, ObjMap* map);

// Removes [key] from [map], if present. Returns the value for the key if found
// or `NULL_VAL` otherwise.
Value wrenMapRemoveKey(WrenVM* vm, ObjMap* map, Value key);

// Creates a new module.
ObjModule* wrenNewModule(WrenVM* vm);

// Creates a new range from [from] to [to].
Value wrenNewRange(WrenVM* vm, double from, double to, bool isInclusive);

// Creates a new string object of [length] and copies [text] into it.
//
// [text] may be NULL if [length] is zero.
Value wrenNewString(WrenVM* vm, const char* text, size_t length);

// Creates a new string object with a buffer large enough to hold a string of
// [length] but does no initialization of the buffer.
//
// The caller is expected to fully initialize the buffer after calling.
Value wrenNewUninitializedString(WrenVM* vm, size_t length);

// Creates a new string that is the concatenation of [left] and [right].
ObjString* wrenStringConcat(WrenVM* vm, const char* left, const char* right);

// Creates a new string containing the code point in [string] starting at byte
// [index]. If [index] points into the middle of a UTF-8 sequence, returns an
// empty string.
Value wrenStringCodePointAt(WrenVM* vm, ObjString* string, int index);

// Creates a new open upvalue pointing to [value] on the stack.
Upvalue* wrenNewUpvalue(WrenVM* vm, Value* value);

// Mark [value] as reachable and still in use. This should only be called
// during the sweep phase of a garbage collection.
void wrenMarkValue(WrenVM* vm, Value value);

// Mark [obj] as reachable and still in use. This should only be called
// during the sweep phase of a garbage collection.
void wrenMarkObj(WrenVM* vm, Obj* obj);

// Releases all memory owned by [obj], including [obj] itself.
void wrenFreeObj(WrenVM* vm, Obj* obj);

// Returns the class of [value].
//
// Unlike wrenGetClassInline in wren_vm.h, this is not inlined. Inlining helps
// performance (significantly) in some cases, but degrades it in others. The
// ones used by the implementation were chosen to give the best results in the
// benchmarks.
ObjClass* wrenGetClass(WrenVM* vm, Value value);

// Returns true if [a] and [b] are strictly the same value. This is identity
// for object values, and value equality for unboxed values.
static inline bool wrenValuesSame(Value a, Value b)
{
#if WREN_NAN_TAGGING
  // Value types have unique bit representations and we compare object types
  // by identity (i.e. pointer), so all we need to do is compare the bits.
  return a == b;
#else
  if (a.type != b.type) return false;
  if (a.type == VAL_NUM) return a.num == b.num;
  return a.obj == b.obj;
#endif
}

// Returns true if [a] and [b] are equivalent. Immutable values (null, bools,
// numbers, ranges, and strings) are equal if they have the same data. All
// other values are equal if they are identical objects.
bool wrenValuesEqual(Value a, Value b);

// TODO: Need to decide if this is for user output of values, or for debug
// tracing.
void wrenPrintValue(Value value);

// Returns true if [value] is a bool. Do not call this directly, instead use
// [IS_BOOL].
static inline bool wrenIsBool(Value value)
{
#if WREN_NAN_TAGGING
  return value == TRUE_VAL || value == FALSE_VAL;
#else
  return value.type == VAL_FALSE || value.type == VAL_TRUE;
#endif
}

// Returns true if [value] is an object of type [type]. Do not call this
// directly, instead use the [IS___] macro for the type in question.
static inline bool wrenIsObjType(Value value, ObjType type)
{
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Converts the raw object pointer [obj] to a [Value].
static inline Value wrenObjectToValue(Obj* obj)
{
#if WREN_NAN_TAGGING
  // The triple casting is necessary here to satisfy some compilers:
  // 1. (uintptr_t) Convert the pointer to a number of the right size.
  // 2. (uint64_t)  Pad it up to 64 bits in 32-bit builds.
  // 3. Or in the bits to make a tagged Nan.
  // 4. Cast to a typedef'd value.
  return (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj));
#else
  Value value;
  value.type = VAL_OBJ;
  value.obj = obj;
  return value;
#endif
}

// Interprets [value] as a [double].
static inline double wrenValueToNum(Value value)
{
#if WREN_NAN_TAGGING
  DoubleBits data;
  data.bits64 = value;
  return data.num;
#else
  return value.num;
#endif
}

// Converts [num] to a [Value].
static inline Value wrenNumToValue(double num)
{
#if WREN_NAN_TAGGING
  DoubleBits data;
  data.num = num;
  return data.bits64;
#else
  return (Value){ VAL_NUM, num, NULL };
#endif
}

#endif
