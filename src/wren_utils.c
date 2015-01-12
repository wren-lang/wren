#include <string.h>

#include "wren_utils.h"
#include "wren_vm.h"

DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, char*);

void wrenSymbolTableInit(WrenVM* vm, SymbolTable* symbols)
{
  wrenStringBufferInit(vm, symbols);
}

void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols)
{
  for (int i = 0; i < symbols->count; i++)
  {
    wrenReallocate(vm, symbols->data[i], 0, 0);
  }

  wrenStringBufferClear(vm, symbols);
}

static int addSymbol(WrenVM* vm, SymbolTable* symbols,
                     const char* name, size_t length)
{
  char* heapString = wrenReallocate(vm, NULL, 0, sizeof(char) * (length + 1));
  strncpy(heapString, name, length);
  heapString[length] = '\0';

  wrenStringBufferWrite(vm, symbols, heapString);
  return symbols->count - 1;
}

int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols, const char* name,
                       size_t length)
{
  // If already present, return an error.
  if (wrenSymbolTableFind(symbols, name, length) != -1) return -1;

  return addSymbol(vm, symbols, name, length);
}

int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols,
                 const char* name, size_t length)
{
  // See if the symbol is already defined.
  int existing = wrenSymbolTableFind(symbols, name, length);
  if (existing != -1) return existing;

  // New symbol, so add it.
  return addSymbol(vm, symbols, name, length);
}

int wrenSymbolTableFind(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  // TODO: O(n). Do something better.
  for (int i = 0; i < symbols->count; i++)
  {
    // TODO: strlen() here is gross. Symbol table should store lengths.
    if (strlen(symbols->data[i]) == length &&
        strncmp(symbols->data[i], name, length) == 0) return i;
  }

  return -1;
}
