#include <string.h>

#include "wren_utils.h"
#include "wren_vm.h"

DEFINE_BUFFER(Byte, uint8_t);
DEFINE_BUFFER(Int, int);
DEFINE_BUFFER(String, ObjString*);

void wrenSymbolTableInit(SymbolTable* symbols)
{
    symbols->hSize = 0;
    symbols->hCapacity = WREN_ST_DEFAULT_CAPACITY;

    // Nothing is set at first, so call calloc() to initialize the bitset
    symbols->bitset = calloc(WREN_ST_DEFAULT_CAPACITY, sizeof(BitSymbol));

    wrenStringBufferInit(&symbols->objs);
}

static void wrenSymbolTableClearBitset(SymbolTable *symbols)
{
    memset(symbols->bitset, 0, symbols->hCapacity * sizeof(BitSymbol));
}

void wrenSymbolTableClear(WrenVM* vm, SymbolTable* symbols)
{
    // Clear the bitset, then the buffer
    wrenSymbolTableClearBitset(symbols);
    wrenStringBufferClear(vm, &symbols->objs);
}

static unsigned long wrenHashDjb2(const char *key, size_t length)
{
    unsigned long hash = 5381; // Magic starting point

    for (size_t i = 0; i < length; i++)
        hash = ((hash << 5) + hash) + key[i];

    return hash;
}

static void wrenSymbolTableGrow(WrenVM *vm, SymbolTable *symbols)
{
    // We need to clear the bitset and re-hash all symbols
    wrenSymbolTableClearBitset(symbols);

    symbols->hCapacity *= 2;
    symbols->bitset = realloc(symbols->bitset, symbols->hCapacity * sizeof(BitSymbol));

    for (size_t i = 0; i < symbols->objs.count; i++)
    {
        size_t newIdx = wrenHashDjb2(symbols->objs.data[i]->value, symbols->objs.data[i]->length) % symbols->hCapacity;

        symbols->bitset[newIdx].set = true;
        symbols->bitset[newIdx].idx = i;
    }
}

int wrenSymbolTableAdd(WrenVM* vm, SymbolTable* symbols,
                       const char* name, size_t length)
{
    ObjString* symbol = AS_STRING(wrenNewStringLength(vm, name, length));

    wrenPushRoot(vm, &symbol->obj);

    // Compute the true index and the hash index
    size_t buffIdx = symbols->objs.count;
    size_t hashIdx = wrenHashDjb2(name, length) % symbols->hCapacity;

    // Locate the first free slot in the bitset
    while (symbols->bitset[hashIdx].set)
        hashIdx++;

    symbols->hSize++;
    if (symbols->hSize == symbols->hCapacity)
        wrenSymbolTableGrow(vm, symbols);

    // "Initialize" the element in the set
    symbols->bitset[hashIdx].set = true;
    symbols->bitset[hashIdx].idx = buffIdx;

    // Add the name to the buffer
    wrenStringBufferWrite(vm, &symbols->objs, symbol);
    wrenPopRoot(vm);

    return buffIdx;
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

int wrenSymbolTableFind(const SymbolTable* symbols,
                        const char* name, size_t length)
{
    size_t hashIdx = wrenHashDjb2(name, length) % symbols->hCapacity;

    while (true)
    {
        // The element does not exist, and there's no collision
        if (!symbols->bitset[hashIdx].set)
            return -1;

        size_t buffIdx = symbols->bitset[hashIdx].idx;
        if (!strncmp(symbols->objs.data[buffIdx]->value, name, length))
            return buffIdx;

        // Maybe there was a collision and we haven't found the correct element
        // yet. Increase the hashIndex and compare the names again
        hashIdx++;
    }

    return -1;
}

void wrenBlackenSymbolTable(WrenVM* vm, SymbolTable* symbolTable)
{
  for (int i = 0; i < symbolTable->objs.count; i++)
  {
    wrenGrayObj(vm, &symbolTable->objs.data[i]->obj);
  }
  
  // Keep track of how much memory is still in use.
  vm->bytesAllocated += symbolTable->objs.capacity * sizeof(*symbolTable->objs.data);
}

int wrenUtf8EncodeNumBytes(int value)
{
  ASSERT(value >= 0, "Cannot encode a negative value.");
  
  if (value <= 0x7f) return 1;
  if (value <= 0x7ff) return 2;
  if (value <= 0xffff) return 3;
  if (value <= 0x10ffff) return 4;
  return 0;
}

int wrenUtf8Encode(int value, uint8_t* bytes)
{
  if (value <= 0x7f)
  {
    // Single byte (i.e. fits in ASCII).
    *bytes = value & 0x7f;
    return 1;
  }
  else if (value <= 0x7ff)
  {
    // Two byte sequence: 110xxxxx 10xxxxxx.
    *bytes = 0xc0 | ((value & 0x7c0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
    return 2;
  }
  else if (value <= 0xffff)
  {
    // Three byte sequence: 1110xxxx 10xxxxxx 10xxxxxx.
    *bytes = 0xe0 | ((value & 0xf000) >> 12);
    bytes++;
    *bytes = 0x80 | ((value & 0xfc0) >> 6);
    bytes++;
    *bytes = 0x80 | (value & 0x3f);
    return 3;
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
    return 4;
  }

  // Invalid Unicode value. See: http://tools.ietf.org/html/rfc3629
  UNREACHABLE();
  return 0;
}

int wrenUtf8Decode(const uint8_t* bytes, uint32_t length)
{
  // Single byte (i.e. fits in ASCII).
  if (*bytes <= 0x7f) return *bytes;

  int value;
  uint32_t remainingBytes;
  if ((*bytes & 0xe0) == 0xc0)
  {
    // Two byte sequence: 110xxxxx 10xxxxxx.
    value = *bytes & 0x1f;
    remainingBytes = 1;
  }
  else if ((*bytes & 0xf0) == 0xe0)
  {
    // Three byte sequence: 1110xxxx	 10xxxxxx 10xxxxxx.
    value = *bytes & 0x0f;
    remainingBytes = 2;
  }
  else if ((*bytes & 0xf8) == 0xf0)
  {
    // Four byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx.
    value = *bytes & 0x07;
    remainingBytes = 3;
  }
  else
  {
    // Invalid UTF-8 sequence.
    return -1;
  }

  // Don't read past the end of the buffer on truncated UTF-8.
  if (remainingBytes > length - 1) return -1;

  while (remainingBytes > 0)
  {
    bytes++;
    remainingBytes--;

    // Remaining bytes must be of form 10xxxxxx.
    if ((*bytes & 0xc0) != 0x80) return -1;

    value = value << 6 | (*bytes & 0x3f);
  }

  return value;
}

int wrenUtf8DecodeNumBytes(uint8_t byte)
{
  // If the byte starts with 10xxxxx, it's the middle of a UTF-8 sequence, so
  // don't count it at all.
  if ((byte & 0xc0) == 0x80) return 0;
  
  // The first byte's high bits tell us how many bytes are in the UTF-8
  // sequence.
  if ((byte & 0xf8) == 0xf0) return 4;
  if ((byte & 0xf0) == 0xe0) return 3;
  if ((byte & 0xe0) == 0xc0) return 2;
  return 1;
}

// From: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2Float
int wrenPowerOf2Ceil(int n)
{
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  
  return n;
}
