#ifndef wren_vm_h
#define wren_vm_h

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_value.h"
#include "wren_utils.h"

// The maximum number of temporary objects that can be made visible to the GC
// at one time.
#define WREN_MAX_TEMP_ROOTS 8

// NOTE: emitOp() assumes it fits in a byte!
typedef enum
{
  #define OPCODE(name, _) CODE_##name,
  #include "wren_opcodes.h"
  #undef OPCODE
} Code;

// A handle to a value, basically just a linked list of extra GC roots.
//
// Note that even non-heap-allocated values can be stored here.
struct WrenHandle
{
  Value value;

  WrenHandle* prev;
  WrenHandle* next;
};

struct WrenVM
{
  ObjClass* boolClass;
  ObjClass* classClass;
  ObjClass* fiberClass;
  ObjClass* fnClass;
  ObjClass* listClass;
  ObjClass* mapClass;
  ObjClass* nullClass;
  ObjClass* numClass;
  ObjClass* objectClass;
  ObjClass* rangeClass;
  ObjClass* stringClass;

  // The fiber that is currently running.
  ObjFiber* fiber;

  // The loaded modules. Each key is an ObjString (except for the core module,
  // whose key is null) for the module's name and the value is the ObjModule
  // for the module.
  ObjMap* modules;
  
  // The most recently imported module. More specifically, the module whose
  // code has most recently finished executing.
  //
  // Not treated like a GC root since the module is already in [modules].
  ObjModule* lastModule;

  // Memory management data:

  // The number of bytes that are known to be currently allocated. Includes all
  // memory that was proven live after the last GC, as well as any new bytes
  // that were allocated since then. Does *not* include bytes for objects that
  // were freed since the last GC.
  size_t bytesAllocated;

  // The number of total allocated bytes that will trigger the next GC.
  size_t nextGC;

  // The first object in the linked list of all currently allocated objects.
  Obj* first;

  // The "gray" set for the garbage collector. This is the stack of unprocessed
  // objects while a garbage collection pass is in process.
  Obj** gray;
  int grayCount;
  int grayCapacity;

  // The list of temporary roots. This is for temporary or new objects that are
  // not otherwise reachable but should not be collected.
  //
  // They are organized as a stack of pointers stored in this array. This
  // implies that temporary roots need to have stack semantics: only the most
  // recently pushed object can be released.
  Obj* tempRoots[WREN_MAX_TEMP_ROOTS];

  int numTempRoots;
  
  // Pointer to the first node in the linked list of active handles or NULL if
  // there are none.
  WrenHandle* handles;
  
  // Pointer to the bottom of the range of stack slots available for use from
  // the C API. During a foreign method, this will be in the stack of the fiber
  // that is executing a method.
  //
  // If not in a foreign method, this is initially NULL. If the user requests
  // slots by calling wrenEnsureSlots(), a stack is created and this is
  // initialized.
  Value* apiStack;

  WrenConfiguration config;
  
  // Compiler and debugger data:

  // The compiler that is currently compiling code. This is used so that heap
  // allocated objects used by the compiler can be found if a GC is kicked off
  // in the middle of a compile.
  Compiler* compiler;

  // There is a single global symbol table for all method names on all classes.
  // Method calls are dispatched directly by index in this table.
  SymbolTable methodNames;

#if WREN_SNAPSHOT
  bool inhibitGC;
#endif
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

// Invoke the finalizer for the foreign object referenced by [foreign].
void wrenFinalizeForeign(WrenVM* vm, ObjForeign* foreign);

// Creates a new [WrenHandle] for [value].
WrenHandle* wrenMakeHandle(WrenVM* vm, Value value);

// Compile [source] in the context of [module] and wrap in a closure that can
// execute it.
//
// Returns NULL if a compile error occurred.
ObjClosure* wrenCompileSource(WrenVM* vm, const char* module,
                              const char* source, bool isExpression,
                              bool printErrors);

// Looks up a variable from a previously-loaded module.
//
// Aborts the current fiber if the module or variable could not be found.
Value wrenGetModuleVariable(WrenVM* vm, Value moduleName, Value variableName);

// Returns the value of the module-level variable named [name] in the [module].
Value wrenFindVariable(ObjModule* module, const char* name);

// Adds a new implicitly declared top-level variable named [name] to [module]
// based on a use site occurring on [line].
//
// Does not check to see if a variable with that name is already declared or
// defined. Returns the symbol for the new variable or -2 if there are too many
// variables defined.
int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length, int line);

// Adds a new top-level variable named [name] to [module], and optionally
// populates line with the line of the implicit first use (line can be NULL).
//
// Returns the symbol for the new variable, -1 if a variable with the given name
// is already defined, or -2 if there are too many variables defined.
// Returns -3 if this is a top-level lowercase variable (localname) that was
// used before being defined.
int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,
                       size_t length, Value value, int* line);

// Pushes [closure] onto [fiber]'s callstack to invoke it. Expects [numArgs]
// arguments (including the receiver) to be on the top of the stack already.
static inline void wrenCallFunction(WrenVM* vm, ObjFiber* fiber,
                                    ObjClosure* closure, int numArgs)
{
  // Grow the call frame array if needed.
  if (fiber->numFrames + 1 > fiber->frameCapacity)
  {
    int max = fiber->frameCapacity * 2;
    fiber->frames = (CallFrame*)wrenReallocate(vm, fiber->frames,
        sizeof(CallFrame) * fiber->frameCapacity, sizeof(CallFrame) * max);
    fiber->frameCapacity = max;
  }
  
  // Grow the stack if needed.
  int stackSize = (int)(fiber->stackTop - fiber->stack);
  int needed = stackSize + closure->fn->maxSlots;
  wrenEnsureStack(vm, fiber, needed);
  
  wrenAppendCallFrame(/* vm, */ fiber, closure, fiber->stackTop - numArgs);
}

// Closes all open upvalues of [fiber].
void closeAllUpvaluesOf(ObjFiber* fiber);

// Marks [obj] as a GC root so that it doesn't get collected.
void wrenPushRoot(WrenVM* vm, Obj* obj);

// Removes the most recently pushed temporary root.
void wrenPopRoot(WrenVM* vm);

// Looks if a char is present in the WREN_SNAPSHOT environment variable.
// It allows to selectively run code related to the snapshot feature.
//
// Handled in test/test.c:
// - 'y': Yes, go into snapshot code; i.e. this is the big Enable button.
// - 'f': Fallthrough into normal execution, after snapshot code.
// Handled elsewhere:
// - 's': Save a snapshot of the current VM.
// --- 'S': The saved snapshot will be verbose (but NOT suitable for restore).
// --- 'n': Save function names.
// --- '1': Save source line for each bytecode.
// - 'r': Restore a snapshot in a transient dedicated empty VM (possibly from
//        the just-saved snapshot, because the order is: save then restore).
// --- 'R': The restore operation will print a lot.
// --- 'x': Execute the restored snapshot, in its transient dedicated VM.
// - '0': Counts of Obj are printed.
// - 'c': Census of Obj are printed.
// - '=': Some "=== step" milestones are printed.
bool wrenSnapshotWant(char c);

// Returns the class of [value].
//
// Defined here instead of in wren_value.h because it's critical that this be
// inlined. That means it must be defined in the header, but the wren_value.h
// header doesn't have a full definition of WrenVM yet.
static inline ObjClass* wrenGetClassInline(WrenVM* vm, Value value)
{
#if WREN_NAN_TAGGING
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value)) return AS_OBJ(value)->classObj;

  switch (GET_TAG(value))
  {
    case TAG_FALSE:     return vm->boolClass;
    case TAG_NAN:       return vm->numClass;
    case TAG_NULL:      return vm->nullClass;
    case TAG_TRUE:      return vm->boolClass;
    case TAG_UNDEFINED: break;
  }
#else
  switch (value.type)
  {
    case VAL_FALSE:     return vm->boolClass;
    case VAL_NULL:      return vm->nullClass;
    case VAL_NUM:       return vm->numClass;
    case VAL_TRUE:      return vm->boolClass;
    case VAL_OBJ:       return AS_OBJ(value)->classObj;
    case VAL_UNDEFINED: break;
  }
#endif

  UNREACHABLE();
  return NULL;
}

// Returns `true` if [name] is a local variable name (starts with a lowercase
// letter).
static inline bool wrenIsLocalName(const char* name)
{
  return name[0] >= 'a' && name[0] <= 'z';
}

static inline bool wrenIsFalsyValue(Value value)
{
  return IS_FALSE(value) || IS_NULL(value);
}

#define DO_ALL_OBJ_TYPES \
  DO(CLASS,    Class   ) \
  DO(CLOSURE,  Closure ) \
  DO(FIBER,    Fiber   ) \
  DO(FN,       Fn      ) \
  DO(FOREIGN,  Foreign ) \
  DO(INSTANCE, Instance) \
  DO(LIST,     List    ) \
  DO(MAP,      Map     ) \
  DO(MODULE,   Module  ) \
  DO(RANGE,    Range   ) \
  DO(STRING,   String  ) \
  DO(UPVALUE,  Upvalue )

typedef uint32_t WrenCount;

typedef struct {
  #define DO(u, l) WrenCount nb##l;
  DO_ALL_OBJ_TYPES
  #undef DO
  WrenCount nb;
} WrenCounts;

typedef struct {
  #define DO(u, l) Obj##l** all##l;
  DO_ALL_OBJ_TYPES
  #undef DO
} WrenCensus;

#undef DO_ALL_OBJ_TYPES

// Count all objects by type.
// The [counts] are expected to be initialized with 0.
void wrenCountAllObj(WrenVM *vm, WrenCounts *counts);

// Allocate memory for [census] arrays, whose sizes are [counts], and perform
// the census.
void wrenCensusAllObj(WrenVM *vm, WrenCounts *counts, WrenCensus *census);

// Free the arrays of a [census].
void wrenFreeCensus(WrenVM *vm, WrenCensus *census);

// Find the [needle] in the [census], whose sizes are [counts].
// (The type of the needle selects the correct array in [census].)
// This yields an identifier independent from the memory allocator.
// The returned value is 1-based.
// Return 0 when not found.
WrenCount wrenFindInCensus(WrenCounts *counts, WrenCensus *census, Obj* needle);

// WrenVM* wrenNewEmptyVM(WrenConfiguration* config);

#endif
