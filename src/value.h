#ifndef wren_value_h
#define wren_value_h

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
  OBJ_STRING
} ObjType;

typedef enum
{
  // The object has been marked during the mark phase of GC.
  FLAG_MARKED = 0x01,
} ObjFlags;

typedef struct sObj
{
  ObjType type;
  ObjFlags flags;

  // The next object in the linked list of all currently allocated objects.
  struct sObj* next;
} Obj;

// TODO(bob): Temp.
typedef struct
{
  ValueType type;
  double num;
  Obj* obj;
} Value;

typedef struct sVM VM;
typedef struct sFiber Fiber;

typedef Value (*Primitive)(VM* vm, Value* args);
typedef void (*FiberPrimitive)(VM* vm, Fiber* fiber, Value* args);

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
  METHOD_BLOCK
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
  // TODO(bob): Hack. Probably don't want to use this much space.
  Method methods[MAX_SYMBOLS];
} ObjClass;

typedef struct
{
  Obj obj;
  ObjClass* classObj;
  // TODO(bob): Fields.
} ObjInstance;

typedef struct
{
  Obj obj;
  char* value;
} ObjString;

// Get the class value of [value] (0 or 1), which must be a boolean.
#define AS_CLASS(value) ((ObjClass*)(value).obj)

// Get the bool value of [obj] (0 or 1), which must be a boolean.
#define AS_BOOL(value) ((value).type == VAL_TRUE)

// Get the function value of [obj] (0 or 1), which must be a function.
#define AS_FN(value) ((ObjFn*)(value).obj)

// Get the double value of [obj], which must be a number.
#define AS_INSTANCE(value) ((ObjInstance*)(value).obj)

// Get the double value of [value], which must be a number.
#define AS_NUM(v) ((v).num)

// Get the const char* value of [v], which must be a string.
#define AS_CSTRING(v) (AS_STRING(v)->value)

// Get the ObjString* of [v], which must be a string.
#define AS_STRING(v) ((ObjString*)(v).obj)

// Determines if [value] is a garbage-collected object or not.
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUM(value) ((value).type == VAL_NUM)
#define IS_BOOL(value) ((value).type == VAL_FALSE || (value).type == VAL_TRUE)

// Returns non-zero if [value] is a function object.
#define IS_FN(value) (valueIsFn(value))

// Returns non-zero if [value] is a string object.
#define IS_STRING(value) (valueIsString(value))

// Convert [obj], an `Obj*`, to a [Value].
#define OBJ_VAL(obj) (objectToValue((Obj*)(obj)))

// Convert [boolean], an int, to a boolean [Value].
#define BOOL_VAL(boolean) (boolean ? TRUE_VAL : FALSE_VAL)

// Convert [n], a raw number, to a [Value].
#define NUM_VAL(n) ((Value){ VAL_NUM, n, NULL })

// TODO(bob): Not C89!
#define FALSE_VAL ((Value){ VAL_FALSE, 0.0, NULL })
#define NULL_VAL ((Value){ VAL_NULL, 0.0, NULL })
#define TRUE_VAL ((Value){ VAL_TRUE, 0.0, NULL })

int valueIsFn(Value value);
int valueIsString(Value value);
Value objectToValue(Obj* obj);

#endif
