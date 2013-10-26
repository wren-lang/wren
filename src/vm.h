#ifndef wren_vm_h
#define wren_vm_h

// TODO(bob): Make these externally controllable.
#define STACK_SIZE 1024
#define MAX_CALL_FRAMES 256
#define MAX_SYMBOLS 256

typedef enum {
  OBJ_NUM,
  OBJ_BLOCK,
  OBJ_CLASS,
  OBJ_INSTANCE
} ObjType;

typedef enum
{
  // The object has been marked during the mark phase of GC.
  FLAG_MARKED = 0x01,
} ObjFlags;

typedef struct
{
  ObjType type;
  ObjFlags flags;
} Obj;

typedef Obj* Value;

typedef struct
{
  Obj obj;
  double value;
} ObjNum;

typedef struct
{
  Obj obj;
  unsigned char* bytecode;
  Value* constants;
  int numConstants;
  int numLocals;
} ObjBlock;

typedef Value (*Primitive)(Value receiver);

typedef enum
{
  METHOD_NONE,
  METHOD_CALL,
  METHOD_PRIMITIVE,
  METHOD_BLOCK
} MethodType;

typedef struct
{
  MethodType type;
  union
  {
    Primitive primitive;
    ObjBlock* block;
  };
} Method;

typedef struct sObjClass
{
  Obj obj;
  struct sObjClass* metaclass;
  // TODO(bob): Hack. Probably don't want to use this much space.
  Method methods[MAX_SYMBOLS];
} ObjClass;

typedef struct
{
  Obj obj;
  ObjClass* classObj;
  // TODO(bob): Fields.
} ObjInstance;

typedef enum
{
  CODE_CONSTANT,
  // Load the constant at index [arg].

  CODE_CLASS,
  // Define a new empty class and push it.

  CODE_METHOD,
  // Add a method for symbol [arg1] with body stored in constant [arg2] to the
  // class on the top of stack. Does not modify the stack.

  CODE_DUP,
  // Push a copy of the top of stack.

  CODE_POP,
  // Pop and discard the top of stack.

  CODE_LOAD_LOCAL,
  // Pushes the value in local slot [arg]. 

  CODE_STORE_LOCAL,
  // Stores the top of stack in local slot [arg]. Does not pop it.

  CODE_LOAD_GLOBAL,
  // Pushes the value in global slot [arg].

  CODE_STORE_GLOBAL,
  // Stores the top of stack in global slot [arg]. Does not pop it.

  CODE_CALL,
  // Invoke the method with symbol [arg].

  CODE_END
  // The current block is done and should be exited.

} Code;

typedef struct
{
  // TODO(bob): Make this dynamically sized.
  char* names[MAX_SYMBOLS];
  int count;
} SymbolTable;

typedef struct
{
  SymbolTable symbols;

  ObjClass* blockClass;
  ObjClass* classClass;
  ObjClass* numClass;

  SymbolTable globalSymbols;
  // TODO(bob): Using a fixed array is gross here.
  Value globals[MAX_SYMBOLS];
  int numGlobals;
} VM;

VM* newVM();
void freeVM(VM* vm);

ObjClass* makeClass();
ObjBlock* makeBlock();
ObjNum* makeNum(double number);
ObjInstance* makeInstance(ObjClass* classObj);

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

Value interpret(VM* vm, ObjBlock* block);

void printValue(Value value);

#endif
