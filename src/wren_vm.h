#ifndef wren_vm_h
#define wren_vm_h

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_value.h"
#include "wren_utils.h"

// In order to token paste __LINE__, you need two weird levels of indirection
// since __LINE__ isn't expanded when used in a token paste.
// See: http://stackoverflow.com/a/1597129/9457
#define WREN_TOKEN_PASTE(a, b) a ## b
#define WREN_TOKEN_PASTE2(a, b) WREN_TOKEN_PASTE(a, b)

// Mark [obj] as a GC root so that it doesn't get collected. Initializes
// [pinned], which must be then passed to [unpinObj].
#define WREN_PIN(vm, obj) \
    do \
    { \
      WrenPinnedObj WREN_TOKEN_PASTE2(wrenPinned, __LINE__); \
      wrenPinObj(vm, (Obj*)obj, &WREN_TOKEN_PASTE2(wrenPinned, __LINE__)); \
    } \
    while (false)

// Remove the most recently pinned object from the list of pinned GC roots.
#define WREN_UNPIN(vm) \
    wrenUnpinObj(vm)

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

  // Pushes the value in the given local slot.
  CODE_LOAD_LOCAL_0,
  CODE_LOAD_LOCAL_1,
  CODE_LOAD_LOCAL_2,
  CODE_LOAD_LOCAL_3,
  CODE_LOAD_LOCAL_4,
  CODE_LOAD_LOCAL_5,
  CODE_LOAD_LOCAL_6,
  CODE_LOAD_LOCAL_7,
  CODE_LOAD_LOCAL_8,

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

  // Pushes the value of the field in slot [arg] of the receiver of the current
  // function. This is used for regular field accesses on "this" directly in
  // methods. This instruction is faster than the more general CODE_LOAD_FIELD
  // instruction.
  CODE_LOAD_FIELD_THIS,

  // Stores the top of the stack in field slot [arg] in the receiver of the
  // current value. Does not pop the value. This instruction is faster than the
  // more general CODE_LOAD_FIELD instruction.
  CODE_STORE_FIELD_THIS,

  // Pops an instance and pushes the value of the field in slot [arg] of it.
  CODE_LOAD_FIELD,

  // Pops an instance and stores the subsequent top of stack in field slot
  // [arg] in it. Does not pop the value.
  CODE_STORE_FIELD,

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

  // Invoke a superclass method with symbol [arg]. The number indicates the
  // number of arguments (not including the receiver).
  CODE_SUPER_0,
  CODE_SUPER_1,
  CODE_SUPER_2,
  CODE_SUPER_3,
  CODE_SUPER_4,
  CODE_SUPER_5,
  CODE_SUPER_6,
  CODE_SUPER_7,
  CODE_SUPER_8,
  CODE_SUPER_9,
  CODE_SUPER_10,
  CODE_SUPER_11,
  CODE_SUPER_12,
  CODE_SUPER_13,
  CODE_SUPER_14,
  CODE_SUPER_15,
  CODE_SUPER_16,

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

  // Create a new list with [arg] elements. The top [arg] values on the stack
  // are the elements in forward order. Removes the elements and then pushes
  // the new list.
  CODE_LIST,

  // Creates a closure for the function stored at [arg] in the constant table.
  //
  // Following the function argument is a number of arguments, two for each
  // upvalue. The first is true if the variable being captured is a local (as
  // opposed to an upvalue), and the second is the index of the local or
  // upvalue being captured.
  //
  // Pushes the created closure.
  CODE_CLOSURE,

  // Creates a class. Top of stack is the superclass, or `null` if the class
  // inherits Object. Below that is a string for the name of the class. Byte
  // [arg] is the number of fields in the class.
  CODE_CLASS,

  // Define a method for symbol [arg]. The class receiving the method is popped
  // off the stack, then the function defining the body is popped.
  CODE_METHOD_INSTANCE,

  // Define a method for symbol [arg]. The class whose metaclass will receive
  // the method is popped off the stack, then the function defining the body is
  // popped.
  CODE_METHOD_STATIC,
  
  // This pseudo-instruction indicates the end of the bytecode. It should
  // always be preceded by a `CODE_RETURN`, so is never actually executed.
  CODE_END
} Code;

// A pinned object is an Obj that has been temporarily made an explicit GC root.
// This is for temporary or new objects that are not otherwise reachable but
// should not be collected.
//
// They are organized as linked list of these objects stored on the stack. The
// WrenVM has a pointer to the head of the list and walks it if a collection
// occurs. This implies that pinned objects need to have stack semantics: only
// the most recently pinned object can be unpinned.
typedef struct sWrenPinnedObj
{
  // The pinned object.
  Obj* obj;

  // The next pinned object.
  struct sWrenPinnedObj* previous;
} WrenPinnedObj;

// TODO: Move into wren_vm.c?
struct WrenVM
{
  // TODO: Use an array for some of these.
  ObjClass* boolClass;
  ObjClass* classClass;
  ObjClass* fiberClass;
  ObjClass* fnClass;
  ObjClass* listClass;
  ObjClass* nullClass;
  ObjClass* numClass;
  ObjClass* objectClass;
  ObjClass* rangeClass;
  ObjClass* stringClass;

  // The currently defined global variables.
  ValueBuffer globals;

  // The fiber that is currently running.
  ObjFiber* fiber;

  // Memory management data:

  // The number of bytes that are known to be currently allocated. Includes all
  // memory that was proven live after the last GC, as well as any new bytes
  // that were allocated since then. Does *not* include bytes for objects that
  // were freed since the last GC.
  size_t bytesAllocated;

  // The number of total allocated bytes that will trigger the next GC.
  size_t nextGC;

  // The minimum value for [nextGC] when recalculated after a collection.
  size_t minNextGC;

  // The scale factor used to calculate [nextGC] from the current number of in
  // use bytes, as a percent. For example, 150 here means that nextGC will be
  // 50% larger than the current number of in-use bytes.
  int heapScalePercent;

  // The first object in the linked list of all currently allocated objects.
  Obj* first;

  // The head of the list of pinned objects. Will be `NULL` if nothing is
  // pinned.
  WrenPinnedObj* pinned;

  // The externally-provided function used to allocate memory.
  WrenReallocateFn reallocate;

  // Foreign function data:

  // During a foreign function call, this will point to the first argument (the
  // receiver) of the call on the fiber's stack.
  Value* foreignCallSlot;

  // During a foreign function call, this will contain the number of arguments
  // to the function.
  int foreignCallNumArgs;

  // Compiler and debugger data:
  
  // The compiler that is currently compiling code. This is used so that heap
  // allocated objects used by the compiler can be found if a GC is kicked off
  // in the middle of a compile.
  Compiler* compiler;

  // There is a single global symbol table for all method names on all classes.
  // Method calls are dispatched directly by index in this table.
  SymbolTable methodNames;

  // Symbol table for the names of all global variables. Indexes here directly
  // correspond to entries in [globals].
  SymbolTable globalNames;
};

// A generic allocation function that handles all explicit memory management.
// It's used like so:
//
// - To allocate new memory, [memory] is NULL and [oldSize] is zero. It should
//   return the allocated memory or NULL on failure.
//
// - To attempt to grow an existing allocation, [memory] is the memory,
//   [oldSize] is its previous size, and [newSize] is the desired size.
//   It should return [memory] if it was able to grow it in place, or a new
//   pointer if it had to move it.
//
// - To shrink memory, [memory], [oldSize], and [newSize] are the same as above
//   but it will always return [memory].
//
// - To free memory, [memory] will be the memory to free and [newSize] and
//   [oldSize] will be zero. It should return NULL.
void* wrenReallocate(WrenVM* vm, void* memory, size_t oldSize, size_t newSize);

// Adds a new global named [name] to the global namespace.
//
// Returns the symbol for the new global, -1 if a global with the given name
// is already defined, or -2 if there are too many globals defined.
int wrenDefineGlobal(WrenVM* vm, const char* name, size_t length, Value value);

// Sets the current Compiler being run to [compiler].
void wrenSetCompiler(WrenVM* vm, Compiler* compiler);

// Mark [obj] as a GC root so that it doesn't get collected. Initializes
// [pinned], which must be then passed to [unpinObj]. This is not intended to be
// used directly. Instead, use the [WREN_PIN_OBJ] macro.
void wrenPinObj(WrenVM* vm, Obj* obj, WrenPinnedObj* pinned);

// Remove the most recently pinned object from the list of pinned GC roots.
// This is not intended to be used directly. Instead, use the [WREN_UNPIN_OBJ]
// macro.
void wrenUnpinObj(WrenVM* vm);

// Returns the class of [value].
//
// Defined here instead of in wren_value.h because it's critical that this be
// inlined. That means it must be defined in the header, but the wren_value.h
// header doesn't have a full definitely of WrenVM yet.
static inline ObjClass* wrenGetClassInline(WrenVM* vm, Value value)
{
#if WREN_NAN_TAGGING
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value)) return AS_OBJ(value)->classObj;

  switch (GET_TAG(value))
  {
    case TAG_FALSE: return vm->boolClass; break;
    case TAG_NAN: return vm->numClass; break;
    case TAG_NULL: return vm->nullClass; break;
    case TAG_TRUE: return vm->boolClass; break;
  }
#else
  switch (value.type)
  {
    case VAL_FALSE: return vm->boolClass;
    case VAL_NULL: return vm->nullClass;
    case VAL_NUM: return vm->numClass;
    case VAL_TRUE: return vm->boolClass;
    case VAL_OBJ: return AS_OBJ(value)->classObj;
  }
#endif

  UNREACHABLE();
  return NULL;
}

#endif
