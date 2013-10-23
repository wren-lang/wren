#ifndef wren_vm_h
#define wren_vm_h

// TODO(bob): Make this externally controllable.
#define STACK_SIZE 1024

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
} Block;

#define MAX_SYMBOLS 256

typedef struct
{
  // TODO(bob): Make this dynamically sized.
  char* symbols[MAX_SYMBOLS];
  int numSymbols;

} VM;

VM* newVM();
void freeVM(VM* vm);

int getSymbol(VM* vm, const char* name, size_t length);

Value interpret(VM* vm, Block* block);

void printValue(Value value);

#endif
