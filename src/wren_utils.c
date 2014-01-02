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
    if (strlen(symbols->names[i]) == length &&
        strncmp(symbols->names[i], name, length) == 0) return i;
  }

  return -1;
}

const char* wrenSymbolTableGetName(SymbolTable* symbols, int symbol)
{
  return symbols->names[symbol];
}

// Most of the logic for working with buffers it element-type independent. To
// avoid stuffing these entire functions in the macro, we'll define a void*
// Buffer type and cast to this. It's important that the fields here line up
// with the type-specific ones defined in wren_utils.h.
typedef struct
{
  void* data;
  int count;
  int capacity;
} Buffer;

static void initBuffer(WrenVM* vm, Buffer* buffer)
{
  buffer->data = NULL;
  buffer->capacity = 0;
  buffer->count = 0;
}

static void writeBuffer(WrenVM* vm, Buffer* buffer, size_t size, void* data)
{
  // Enlarge the buffer if needed
  if (buffer->capacity < buffer->count + 1)
  {
    // Give it an initial bump, then double each time.
    int capacity = buffer->capacity == 0 ? 8 : buffer->capacity * 2;
    buffer->data = wrenReallocate(vm, buffer->data,
                                  buffer->capacity * size, capacity * size);
    // TODO: Handle allocation failure.
    buffer->capacity = capacity;
  }

  memcpy(buffer->data + buffer->count * size, data, size);
  buffer->count++;
}

#define DEFINE_BUFFER(name, type) \
    void wren##name##BufferInit(WrenVM* vm, name##Buffer* buffer) \
    { \
      initBuffer(vm, (Buffer*)buffer); \
    } \
    \
    void wren##name##BufferClear(WrenVM* vm, name##Buffer* buffer) \
    { \
      wrenReallocate(vm, buffer->data, 0, 0); \
      wren##name##BufferInit(vm, buffer); \
    } \
    void wren##name##BufferWrite(WrenVM* vm, name##Buffer* buffer, type data) \
    { \
      writeBuffer(vm, (Buffer*)buffer, sizeof(type), &data); \
    }

DEFINE_BUFFER(Byte, uint8_t)
DEFINE_BUFFER(Int, int)

/*
void wrenBufferInit(WrenVM* vm, Buffer* buffer)
{
  buffer->data = NULL;
  buffer->capacity = 0;
  buffer->count = 0;
}

void wrenBufferWriteElement(WrenVM* vm, Buffer* buffer, void* data, size_t size)
{
  // Enlarge the buffer if needed
  if (buffer->capacity < buffer->count + 1)
  {
    // Give it an initial bump, then double each time.
    int capacity = buffer->capacity == 0 ? 8 : buffer->capacity * 2;
    buffer->data = wrenReallocate(vm, buffer->data,
                                  buffer->capacity * size, capacity * size);
    // TODO: Handle allocation failure.
    buffer->capacity = capacity;
  }

  memcpy(buffer->data + buffer->count * size, data, size);
  buffer->count++;
}

void wrenBufferClear(WrenVM* vm, Buffer* buffer)
{
  wrenReallocate(vm, buffer->data, 0, 0);
  wrenBufferInit(vm, buffer);
}
*/
