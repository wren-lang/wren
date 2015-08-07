#ifndef wren_vm_h
#define wren_vm_h

#include "wren_common.h"
#include "wren_compiler.h"
#include "wren_value.h"
#include "wren_utils.h"

// The maximum number of temporary objects that can be made visible to the GC
// at one time.
#define WREN_MAX_TEMP_ROOTS 5

typedef enum
{
  #define OPCODE(name) CODE_##name,
  #include "wren_opcodes.h"
  #undef OPCODE
} Code;

// A handle to a method.
//
// It is a node in the doubly-linked list of currently allocate method handles.
// Each node has a reference to the fiber containing the method stub to call
// the method.
struct WrenMethod
{
  // The fiber that invokes the method. Its stack is pre-populated with the
  // receiver for the method, and it contains a single callframe whose function
  // is the bytecode stub to invoke the method.
  ObjFiber* fiber;

  WrenMethod* prev;
  WrenMethod* next;
};

// A handle to a value, basically just a linked list of extra GC roots.
//
// Note that even non-heap-allocated values can be stored here.
struct WrenValue
{
  Value value;
  
  WrenValue* prev;
  WrenValue* next;
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

  // The loaded modules. Each key is an ObjString (except for the main module,
  // whose key is null) for the module's name and the value is the ObjModule
  // for the module.
  ObjMap* modules;

  // The ID that will be assigned to the next fiber that is allocated. Fibers
  // are given unique-ish (not completely unique since this can overflow) IDs
  // so that they can be used as map keys.
  uint16_t nextFiberId;

  // Memory management data:

  // The externally-provided function used to allocate memory.
  WrenReallocateFn reallocate;

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

  // The list of temporary roots. This is for temporary or new objects that are
  // not otherwise reachable but should not be collected.
  //
  // They are organized as a stack of pointers stored in this array. This
  // implies that temporary roots need to have stack semantics: only the most
  // recently pushed object can be released.
  Obj* tempRoots[WREN_MAX_TEMP_ROOTS];

  int numTempRoots;

  // Foreign function data:

  // During a foreign function call, this will point to the first argument (the
  // receiver) of the call on the fiber's stack.
  Value* foreignCallSlot;

  // Pointer to the first node in the linked list of active method handles or
  // NULL if there are no handles.
  WrenMethod* methodHandles;
  
  // Pointer to the first node in the linked list of active value handles or
  // NULL if there are no handles.
  WrenValue* valueHandles;

  // During a foreign function call, this will contain the number of arguments
  // to the function.
  int foreignCallNumArgs;

  // The function used to locate foreign functions.
  WrenBindForeignMethodFn bindForeign;

  // The function used to load modules.
  WrenLoadModuleFn loadModule;

  // Compiler and debugger data:

  // The compiler that is currently compiling code. This is used so that heap
  // allocated objects used by the compiler can be found if a GC is kicked off
  // in the middle of a compile.
  Compiler* compiler;

  // There is a single global symbol table for all method names on all classes.
  // Method calls are dispatched directly by index in this table.
  SymbolTable methodNames;
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

// Looks up the core module in the module map.
ObjModule* wrenGetCoreModule(WrenVM* vm);

// Imports the module with [name].
//
// If the module has already been imported (or is already in the middle of
// being imported, in the case of a circular import), returns true. Otherwise,
// returns a new fiber that will execute the module's code. That fiber should
// be called before any variables are loaded from the module.
//
// If the module could not be found, returns false.
Value wrenImportModule(WrenVM* vm, const char* name);

// Returns the value of the module-level variable named [name] in the main
// module.
Value wrenFindVariable(WrenVM* vm, ObjModule* module, const char* name);

// Adds a new implicitly declared top-level variable named [name] to [module].
//
// Does not check to see if a variable with that name is already declared or
// defined. Returns the symbol for the new variable or -2 if there are too many
// variables defined.
int wrenDeclareVariable(WrenVM* vm, ObjModule* module, const char* name,
                        size_t length);

// Adds a new top-level variable named [name] to [module].
//
// Returns the symbol for the new variable, -1 if a variable with the given name
// is already defined, or -2 if there are too many variables defined.
int wrenDefineVariable(WrenVM* vm, ObjModule* module, const char* name,
                       size_t length, Value value);

// Sets the current Compiler being run to [compiler].
void wrenSetCompiler(WrenVM* vm, Compiler* compiler);

// Mark [obj] as a GC root so that it doesn't get collected.
void wrenPushRoot(WrenVM* vm, Obj* obj);

// Remove the most recently pushed temporary root.
void wrenPopRoot(WrenVM* vm);

// Returns the class of [value].
//
// Defined here instead of in wren_value.h because it's critical that this be
// inlined. That means it must be defined in the header, but the wren_value.h
// header doesn't have a full definitely of WrenVM yet.
static inline ObjClass* wrenGetClassInline(WrenVM* vm, Value value)
{
  if (IS_NUM(value)) return vm->numClass;
  if (IS_OBJ(value)) return AS_OBJ(value)->classObj;

#if WREN_NAN_TAGGING
  switch (GET_TAG(value))
  {
    case TAG_FALSE:     return vm->boolClass; break;
    case TAG_NAN:       return vm->numClass; break;
    case TAG_NULL:      return vm->nullClass; break;
    case TAG_TRUE:      return vm->boolClass; break;
    case TAG_UNDEFINED: UNREACHABLE();
  }
#else
  switch (value.type)
  {
    case VAL_FALSE:     return vm->boolClass;
    case VAL_NULL:      return vm->nullClass;
    case VAL_NUM:       return vm->numClass;
    case VAL_TRUE:      return vm->boolClass;
    case VAL_OBJ:       return AS_OBJ(value)->classObj;
    case VAL_UNDEFINED: UNREACHABLE();
  }
#endif

  UNREACHABLE();
}

#endif
