#include <string.h>

#include "wren_utils.h"
#include "wren_vm.h"

DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, String);

void wrenSymbolTableInit(SymbolTable* symbols)
{
  wrenStringBufferInit(symbols);
}

void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols)
{
  for (int i = 0; i < symbols->count; i++)
  {
    DEALLOCATE(vm, symbols->data[i].buffer);
  }

  wrenStringBufferClear(vm, symbols);
}

int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols,
                       const char* name, size_t length)
{
  String symbol;
  symbol.buffer = ALLOCATE_ARRAY(vm, char, length + 1);
  memcpy(symbol.buffer, name, length);
  symbol.buffer[length] = '\0';
  symbol.length = (int)length;

  wrenStringBufferWrite(vm, symbols, symbol);
  return symbols->count - 1;
}

int wrenSymbolTableEnsure(WrenVM* vm, SymbolTable* symbols,
                          const char* name, size_t length)
{
  // See if the symbol is already defined.
  int existing = wrenSymbolTableFind(symbols, name, length);
  if (existing != -1) return existing;

  // New symbol, so add it.
  return wrenSymbolTableAdd(vm, symbols, name, length);
}

int wrenSymbolTableFind(SymbolTable* symbols, const char* name, size_t length)
{
  // See if the symbol is already defined.
  // TODO: O(n). Do something better.
  for (int i = 0; i < symbols->count; i++)
  {
    if (symbols->data[i].length == length &&
        memcmp(symbols->data[i].buffer, name, length) == 0) return i;
  }

  return -1;
}

int wrenUtf8NumBytes(int value)
{
  ASSERT(value >= 0, "Cannot encode a negative value.");
  
  if (value <= 0x7f) return 1;
  if (value <= 0x7ff) return 2;
  if (value <= 0xffff) return 3;
  if (value <= 0x10ffff) return 4;
  return 0;
}

void wrenUtf8Encode(int value, uint8_t* bytes)
{
  if (value <= 0x7f)
  {
    // Single byte (i.e. fits in ASCII).
    *bytes = value & 0x7f;
  }
  else if (value <= 0x7ff)
  {
    // Two byte sequence: 110xxxxx	 10xxxxxx.
    *bytes = 0xc0 | ((value & 0x7c0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
  }
  else if (value <= 0xffff)
  {
    // Three byte sequence: 1110xxxx	 10xxxxxx 10xxxxxx.
    *bytes = 0xe0 | ((value & 0xf000) >> 12);
    bytes++;
    *bytes = 0x80 | ((value & 0xfc0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
  }
  else if (value <= 0x10ffff)
  {
    // Four byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
    *bytes = 0xf0 | ((value & 0x1c0000) >> 18);
    bytes++;
    *bytes = 0x80 | ((value & 0x3f000) >> 12);
    bytes++;
    *bytes = 0x80 | ((value & 0xfc0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
  }
  else
  {
    // Invalid Unicode value. See: http://tools.ietf.org/html/rfc3629
    ASSERT(false, "Invalid UTF-8 value.");
  }
}
