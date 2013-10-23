#ifndef wren_vm_h
#define wren_vm_h

// TODO(bob): Make these externally controllable.
#define STACK_SIZE 1024
#define MAX_CALL_FRAMES 256


typedef enum {
  OBJ_INT
} ObjType;

typedef enum
{
  // The object has been marked during the mark phase of GC.
  FLAG_MARKED = 0x01,
} ObjFlags;

typedef struct sObj {
  ObjType type;
  ObjFlags flags;

  union {
    /* OBJ_INT */
    int value;
  };
} Obj;

typedef Obj* Value;

typedef enum
{
  CODE_CONSTANT,
  // Load the constant at index [arg].

  CODE_POP,
  // Pop and discard the top of stack.

  CODE_LOAD_LOCAL,
  // Pushes the value in local slot [arg]. 

  CODE_STORE_LOCAL,
  // Pops and stores the value in local slot [arg].

  CODE_CALL,
  // Invoke the method with symbol [arg].

  CODE_END
  // The current block is done and should be exited.

} Code;

typedef struct
{
  unsigned char* bytecode;
  Value* constants;
  int numConstants;
  int numLocals;
} Block;

#define MAX_SYMBOLS 256

typedef Value (*Primitive)(Value receiver);

typedef struct
{
  // TODO(bob): Make this dynamically sized.
  char* names[MAX_SYMBOLS];
  int count;
} SymbolTable;

typedef struct
{
  // TODO(bob): Hack. Probably don't want to use this much space.
  Primitive methods[MAX_SYMBOLS];
} Class;

typedef struct
{
  SymbolTable symbols;
  Class numClass;
} VM;

VM* newVM();
void freeVM(VM* vm);

Value makeNum(int number);

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

Value interpret(VM* vm, Block* block);

void printValue(Value value);

#endif
