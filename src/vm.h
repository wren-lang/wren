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

typedef struct
{
  Value stack[STACK_SIZE];
  int stackSize;
} Fiber;

typedef enum
{
  CODE_CONSTANT,
  // Load the constant at index [arg].

  CODE_END
  // The current block is done and should be exited.

} Code;

typedef struct
{
  unsigned char* bytecode;
  Value* constants;
  int numConstants;
} Block;

Fiber* newFiber();
Value interpret(Fiber* fiber, Block* block);

void printValue(Value value);

#endif
