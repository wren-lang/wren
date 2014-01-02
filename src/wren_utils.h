#ifndef wren_utils_h
#define wren_utils_h

#include "wren.h"
#include "wren_common.h"

// Reusable data structures and other utility functions.

typedef struct
{
  // TODO: Make this dynamically sized.
  char* names[MAX_SYMBOLS];
  int count;
} SymbolTable;

// Initializes the symbol table.
void wrenSymbolTableInit(SymbolTable* symbols);

// Frees all dynamically allocated memory used by the symbol table, but not the
// SymbolTable itself.
void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols);

// Adds name to the symbol table. Returns the index of it in the table. Returns
// -1 if the symbol is already present.
int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols,
              const char* name, size_t length);

// Adds name to the symbol table. Returns the index of it in the table. Will
// use an existing symbol if already present.
int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols,
                 const char* name, size_t length);

// Looks up name in the symbol table. Returns its index if found or -1 if not.
int wrenSymbolTableFind(SymbolTable* symbols, const char* name, size_t length);

// Given an index in the symbol table, returns its name.
const char* wrenSymbolTableGetName(SymbolTable* symbols, int symbol);

// We need buffers of a few different types. To avoid lots of casting between
// void* and back, we'll use the preprocessor as a poor man's generics and let
// it generate a few type-specific ones.
#define DECLARE_BUFFER(name, type) \
    typedef struct \
    { \
      type* data; \
      int count; \
      int capacity; \
    } name##Buffer; \
    \
    void wren##name##BufferInit(WrenVM* vm, name##Buffer* buffer); \
    void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer); \
    void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data)

DECLARE_BUFFER(Byte, uint8_t);
DECLARE_BUFFER(Int, int);

#endif
