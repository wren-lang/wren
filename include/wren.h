#ifndef wren_h
#define wren_h

#include <stdlib.h>

typedef struct WrenVM WrenVM;

// A generic allocation that handles all explicit memory management used by
// Wren. It's used like so:
//
// - To allocate new memory, [memory] is NULL and [oldSize] is zero.
//
// - To attempt to grow an existing allocation, [memory] is the memory,
//   [oldSize] is its previous size, and [newSize] is the desired size.
//   It returns [memory] if it was able to grow it in place, or a new pointer
//   if it had to move it.
//
// - To shrink memory, [memory], [oldSize], and [newSize] are the same as above
//   but it will always return [memory]. If [newSize] is zero, the memory will
//   be freed and `NULL` will be returned.
//
// - To free memory, [newSize] will be zero.
typedef void* (*WrenReallocateFn)(void* memory, size_t oldSize, size_t newSize);

// Creates a new Wren virtual machine. It allocates memory for the VM itself
// using [reallocateFn] and then uses that throughout its lifetime to manage
// memory.
WrenVM* wrenNewVM(WrenReallocateFn reallocateFn);

// Disposes of all resources is use by [vm], which was previously created by a
// call to [wrenNewVM].
void wrenFreeVM(WrenVM* vm);

// Runs [source], a string of Wren source code in a new fiber in [vm]. Returns
// zero if successful.
// TODO(bob): Define error codes.
int wrenInterpret(WrenVM* vm, const char* source);

#endif
