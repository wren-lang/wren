#include <string.h>

#include "wren_utils.h"
#include "wren_vm.h"

void wrenSymbolTableInit(SymbolTable* symbols)
{
  symbols->count = 0;
}

void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols)
{
  for (int i = 0; i < symbols->count; i++)
  {
    wrenReallocate(vm, symbols->names[i], 0, 0);
  }
}

static int addSymbol(WrenVM* vm, SymbolTable* symbols,
                     const char* name, size_t length)
{
  symbols->names[symbols->count] = wrenReallocate(vm, NULL, 0,
                                                  sizeof(char) * (length + 1));
  strncpy(symbols->names[symbols->count], name, length);
  symbols->names[symbols->count][length] = '\0';

  return symbols->count++;
}

int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols, const char* name, size_t length)
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
    if (strlen(symbols->names[i]) == length &&
        strncmp(symbols->names[i], name, length) == 0) return i;
  }

  return -1;
}

const char* wrenSymbolTableGetName(SymbolTable* symbols, int symbol)
{
  return symbols->names[symbol];
}
