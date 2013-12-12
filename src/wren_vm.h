#ifndef wren_vm_h
#define wren_vm_h

#include "wren_common.h"
#include "wren_value.h"

// TODO(bob): Make these externally controllable.
#define STACK_SIZE 1024
#define MAX_CALL_FRAMES 256

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

  // Create a new list with [arg] elements. The top [arg] values on the stack
  // are the elements in forward order. Removes the elements and then pushes
  // the new list.
  CODE_LIST,

  // Creates a closure for the function stored at [arg] in the constant table.
  //
  // Following the function argument is a number of arguments, two for each
  // upvalue. The first is non-zero if the variable being captured is a local
  // (as opposed to an upvalue), and the second is the index of the local or
  // upvalue being captured.
  //
  // Pushes the created closure.
  CODE_CLOSURE,

  // Pushes the value in local slot [arg].
  CODE_LOAD_LOCAL,

  // Stores the top of stack in local slot [arg]. Does not pop it.
  CODE_STORE_LOCAL,

  // Pushes the value in upvalue [arg].
  CODE_LOAD_UPVALUE,

  // Stores the top of stack in upvalue [arg]. Does not pop it.
  CODE_STORE_UPVALUE,

  // Pushes the value in global slot [arg].
  CODE_LOAD_GLOBAL,

  // Stores the top of stack in global slot [arg]. Does not pop it.
  CODE_STORE_GLOBAL,

  // Pushes the value of the field in slot [arg] for the current receiver.
  CODE_LOAD_FIELD,

  // Stores the top of stack in field slot [arg] in the current receiver.
  CODE_STORE_FIELD,

  // Push a copy of the top of stack.
  CODE_DUP,

  // Pop and discard the top of stack.
  CODE_POP,

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
  CODE_CALL_11,
  CODE_CALL_12,
  CODE_CALL_13,
  CODE_CALL_14,
  CODE_CALL_15,
  CODE_CALL_16,

  // Jump the instruction pointer [arg] forward.
  CODE_JUMP,

  // Jump the instruction pointer [arg] backward. Pop and discard the top of
  // the stack.
  CODE_LOOP,

  // Pop and if not truthy then jump the instruction pointer [arg] forward.
  CODE_JUMP_IF,

  // If the top of the stack is false, jump [arg] forward. Otherwise, pop and
  // continue.
  CODE_AND,

  // If the top of the stack is non-false, jump [arg] forward. Otherwise, pop
  // and continue.
  CODE_OR,

  // Pop [a] then [b] and push true if [b] is an instance of [a].
  CODE_IS,

  // Close the upvalue for the local on the top of the stack, then pop it.
  CODE_CLOSE_UPVALUE,

  // Exit from the current function and return the value on the top of the
  // stack.
  CODE_RETURN,

  // Define a new empty class and push it.
  CODE_CLASS,

  // Pop a superclass off the stack, then push a new class that extends it.
  CODE_SUBCLASS,

  // Define a method for symbol [arg] whose body is the function popped off the
  // top of the stack. The class receiving the method is on top of the stack
  // (after the function is popped off) and remains after this.
  CODE_METHOD_INSTANCE,

  // Define a method for symbol [arg] whose body is the function popped off the
  // top of the stack. The class receiving the method is the metaclass of the
  // class on top of the stack (after the function is popped off) and remains
  // after this.
  CODE_METHOD_STATIC,

  // Define a constructor method for symbol [arg] whose body is the function
  // popped off the top of the stack. The class receiving the method is the
  // metaclass of the class on top of the stack (after the function is popped
  // off) and remains after this.
  CODE_METHOD_CTOR,

  // This pseudo-instruction indicates the end of the bytecode. It should
  // always be preceded by a `CODE_RETURN`, so is never actually executed.
  CODE_END
} Code;

typedef struct
{
  // TODO(bob): Make this dynamically sized.
  char* names[MAX_SYMBOLS];
  int count;
} SymbolTable;

// A pinned object is an Obj that has been temporarily made an explicit GC root.
// This is for temporary or new objects that are not otherwise reachable but
// should not be collected.
//
// They are organized as linked list of these objects stored on the stack. The
// WrenVM has a pointer to the head of the list and walks it if a collection
// occurs. This implies that pinned objects need to have stack semantics: only
// the most recently pinned object can be unpinned.
// TODO(bob): Move into wren_vm.c.
typedef struct sPinnedObj
{
  // The pinned object.
  Obj* obj;

  // The next pinned object.
  struct sPinnedObj* previous;
} PinnedObj;

// TODO(bob): Move into wren_vm.c?
struct WrenVM
{
  SymbolTable methods;

  ObjClass* boolClass;
  ObjClass* classClass;
  ObjClass* fnClass;
  ObjClass* listClass;
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

  // The head of the list of pinned objects. Will be `NULL` if nothing is
  // pinned.
  PinnedObj* pinned;

  // The externally-provided function used to allocate memory.
  WrenReallocateFn reallocate;
};

// TODO(bob): Move into wren_vm.c.
typedef struct
{
  // Index of the current (really next-to-be-executed) instruction in the
  // block's bytecode.
  int ip;

  // The function or closure being executed.
  Value fn;

  // Index of the first stack slot used by this call frame. This will contain
  // the receiver, followed by the function's parameters, then local variables
  // and temporaries.
  int stackStart;
} CallFrame;

// TODO(bob): Move into wren_vm.c.
struct sFiber
{
  Value stack[STACK_SIZE];
  int stackSize;

  CallFrame frames[MAX_CALL_FRAMES];
  int numFrames;

  // Pointer to the first node in the linked list of open upvalues that are
  // pointing to values still on the stack. The head of the list will be the
  // upvalue closest to the top of the stack, and then the list works downwards.
  Upvalue* openUpvalues;
};

void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize);

// TODO(bob): Make these static or prefix their names.

// Initializes the symbol table.
void initSymbolTable(SymbolTable* symbols);

// Removes any symbols added after [count] symbols were defined.
void truncateSymbolTable(SymbolTable* symbols, int count);

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
Value findGlobal(WrenVM* vm, const char* name);

Value interpret(WrenVM* vm, Value function);

// Pushes [function] onto [fiber]'s callstack and invokes it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
// [function] can be an `ObjFn` or `ObjClosure`.
void wrenCallFunction(Fiber* fiber, Value function, int numArgs);

// Mark [obj] as a GC root so that it doesn't get collected. Initializes
// [pinned], which must be then passed to [unpinObj].
void pinObj(WrenVM* vm, Obj* obj, PinnedObj* pinned);

// Remove the most recently pinned object from the list of pinned GC roots.
void unpinObj(WrenVM* vm);

#endif
