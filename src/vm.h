#ifndef wren_vm_h
#define wren_vm_h

// TODO(bob): Make these externally controllable.
#define STACK_SIZE 1024
#define MAX_CALL_FRAMES 256
#define MAX_SYMBOLS 256
#define MAX_PINNED 16

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

// TODO(bob): Evaluating value here twice sucks.
#define IS_FN(value) (IS_OBJ(value) && (value).obj->type == OBJ_FN)
#define IS_STRING(value) (IS_OBJ(value) && (value).obj->type == OBJ_STRING)

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

typedef Value (*Primitive)(VM* vm, Fiber* fiber, Value* args);

typedef struct
{
  Obj obj;
  unsigned char* bytecode;
  Value* constants;
  int numConstants;
} ObjFn;

typedef enum
{
  METHOD_NONE,
  METHOD_PRIMITIVE,
  METHOD_BLOCK
} MethodType;

typedef struct
{
  MethodType type;
  union
  {
    Primitive primitive;
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

typedef enum
{
  // Load the constant at index [arg].
  CODE_CONSTANT,

  // Push null onto the stack.
  CODE_NULL,

  // Push false onto the stack.
  CODE_FALSE,

  // Push true onto the stack.
  CODE_TRUE,

  // Define a new empty class and push it.
  CODE_CLASS,

  // Pop a superclass off the stack, then push a new class that extends it.
  CODE_SUBCLASS,

  // Push the metaclass of the class on the top of the stack. Does not discard
  // the class.
  CODE_METACLASS,

  // Add a method for symbol [arg1] with body stored in constant [arg2] to the
  // class on the top of stack. Does not modify the stack.
  CODE_METHOD,

  // Push a copy of the top of stack.
  CODE_DUP,

  // Pop and discard the top of stack.
  CODE_POP,

  // Pushes the value in local slot [arg].
  CODE_LOAD_LOCAL,

  // Stores the top of stack in local slot [arg]. Does not pop it.
  CODE_STORE_LOCAL,

  // Pushes the value in global slot [arg].
  CODE_LOAD_GLOBAL,

  // Stores the top of stack in global slot [arg]. Does not pop it.
  CODE_STORE_GLOBAL,

  // Invoke the method with symbol [arg]. The number indicates the number of
  // arguments (not including the receiver).
  CODE_CALL_0,
  CODE_CALL_1,
  CODE_CALL_2,
  CODE_CALL_3,
  CODE_CALL_4,
  CODE_CALL_5,
  CODE_CALL_6,
  CODE_CALL_7,
  CODE_CALL_8,
  CODE_CALL_9,
  CODE_CALL_10,

  // Jump the instruction pointer [arg1] forward.
  CODE_JUMP,

  // Pop and if not truthy then jump the instruction pointer [arg1] forward.
  CODE_JUMP_IF,

  // Pop [a] then [b] and push true if [b] is an instance of [a].
  CODE_IS,

  // The current block is done and should be exited.
  CODE_END
} Code;

typedef struct
{
  // TODO(bob): Make this dynamically sized.
  char* names[MAX_SYMBOLS];
  int count;
} SymbolTable;

struct sVM
{
  SymbolTable methods;

  ObjClass* boolClass;
  ObjClass* classClass;
  ObjClass* fnClass;
  ObjClass* nullClass;
  ObjClass* numClass;
  ObjClass* objectClass;
  ObjClass* stringClass;

  // The singleton values.
  Value unsupported;

  SymbolTable globalSymbols;
  // TODO(bob): Using a fixed array is gross here.
  Value globals[MAX_SYMBOLS];

  // TODO(bob): Support more than one fiber.
  Fiber* fiber;

  // Memory management data:

  // How many bytes of object data have been allocated so far.
  size_t totalAllocated;

  // The number of total allocated bytes that will trigger the next GC.
  size_t nextGC;

  // The first object in the linked list of all currently allocated objects.
  Obj* first;

  // How many pinned objects are in the stack.
  int numPinned;

  // The stack "pinned" objects. These are temporary explicit GC roots so that
  // the collector can find them before they are reachable through a normal
  // root.
  Obj* pinned[MAX_PINNED];
};

typedef struct
{
  // Index of the current (really next-to-be-executed) instruction in the
  // block's bytecode.
  int ip;

  // The function being executed.
  ObjFn* fn;

  // Index of the first stack slot used by this call frame. This will contain
  // the receiver, followed by the function's parameters, then local variables
  // and temporaries.
  int stackStart;
} CallFrame;

struct sFiber
{
  Value stack[STACK_SIZE];
  int stackSize;

  CallFrame frames[MAX_CALL_FRAMES];
  int numFrames;
};

// TODO(bob): Temp.
#define OBJ_VAL(obj) (objectToValue((Obj*)(obj)))
Value objectToValue(Obj* obj);

// TODO(bob): Not C89!
#define FALSE_VAL ((Value){ VAL_FALSE, 0.0, NULL })
#define NULL_VAL ((Value){ VAL_NULL, 0.0, NULL })
#define TRUE_VAL ((Value){ VAL_TRUE, 0.0, NULL })
// TODO(bob): Gross.
#define NO_VAL ((Value){ VAL_OBJ, 0.0, NULL })

#define BOOL_VAL(b) (b ? TRUE_VAL : FALSE_VAL)
#define NUM_VAL(n) ((Value){ VAL_NUM, n, NULL })

VM* newVM();
void freeVM(VM* vm);

// Creates a new function object. Assumes the compiler will fill it in with
// bytecode, constants, etc.
ObjFn* newFunction(VM* vm);

// Creates a new class object.
ObjClass* newClass(VM* vm, ObjClass* superclass);

// Creates a new instance of the given [classObj].
Value newInstance(VM* vm, ObjClass* classObj);

// Creates a new string object and copies [text] into it.
Value newString(VM* vm, const char* text, size_t length);

// Initializes the symbol table.
void initSymbolTable(SymbolTable* symbols);

// Frees all dynamically allocated memory used by the symbol table, but not the
// SymbolTable itself.
void clearSymbolTable(SymbolTable* symbols);

// Adds name to the symbol table. Returns the index of it in the table. Returns
// -1 if the symbol is already present.
int addSymbol(SymbolTable* symbols, const char* name, size_t length);

// Adds name to the symbol table. Returns the index of it in the table. Will
// use an existing symbol if already present.
int ensureSymbol(SymbolTable* symbols, const char* name, size_t length);

// Looks up name in the symbol table. Returns its index if found or -1 if not.
int findSymbol(SymbolTable* symbols, const char* name, size_t length);

// Given an index in the symbol table, returns its name.
const char* getSymbolName(SymbolTable* symbols, int symbol);

// Returns the global variable named [name].
Value findGlobal(VM* vm, const char* name);

Value interpret(VM* vm, ObjFn* fn);

// Push [fn] onto [fiber]'s callstack and invoke it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
void callFunction(Fiber* fiber, ObjFn* fn, int numArgs);

void printValue(Value value);

// Mark [obj] as a GC root so that it doesn't get collected.
void pinObj(VM* vm, Obj* obj);

// Unmark [obj] as a GC root so that it doesn't get collected.
void unpinObj(VM* vm, Obj* obj);

#endif
