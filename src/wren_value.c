#include <stdio.h>
#include <string.h>

#include "wren.h"
#include "wren_value.h"
#include "wren_vm.h"

// TODO: Tune these.
// The initial (and minimum) capacity of a non-empty list or map object.
#define MIN_CAPACITY 16

// The rate at which a collection's capacity grows when the size exceeds the
// current capacity. The new capacity will be determined by *multiplying* the
// old capacity by this. Growing geometrically is necessary to ensure that
// adding to a collection has O(1) amortized complexity.
#define GROW_FACTOR 2

// The maximum percentage of map entries that can be filled before the map is
// grown. A lower load takes more memory but reduces collisions which makes
// lookup faster.
#define MAP_LOAD_PERCENT 75

// Hash codes for singleton values.
// TODO: Tune these.
#define HASH_FALSE 1
#define HASH_NAN   2
#define HASH_NULL  3
#define HASH_TRUE  4

DEFINE_BUFFER(Value, Value);
DEFINE_BUFFER(Method, Method);

#define ALLOCATE(vm, type) \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type)))

#define ALLOCATE_FLEX(vm, mainType, arrayType, count) \
    ((mainType*)wrenReallocate(vm, NULL, 0, \
        sizeof(mainType) + sizeof(arrayType) * count))

#define ALLOCATE_ARRAY(vm, type, count) \
    ((type*)wrenReallocate(vm, NULL, 0, sizeof(type) * count))

static void initObj(WrenVM* vm, Obj* obj, ObjType type, ObjClass* classObj)
{
  obj->type = type;
  obj->marked = false;
  obj->classObj = classObj;
  obj->next = vm->first;
  vm->first = obj;
}

ObjClass* wrenNewSingleClass(WrenVM* vm, int numFields, ObjString* name)
{
  ObjClass* classObj = ALLOCATE(vm, ObjClass);
  initObj(vm, &classObj->obj, OBJ_CLASS, NULL);
  classObj->superclass = NULL;
  classObj->numFields = numFields;
  classObj->name = name;

  wrenPushRoot(vm, (Obj*)classObj);
  wrenMethodBufferInit(vm, &classObj->methods);
  wrenPopRoot(vm);

  return classObj;
}

void wrenBindSuperclass(WrenVM* vm, ObjClass* subclass, ObjClass* superclass)
{
  ASSERT(superclass != NULL, "Must have superclass.");

  subclass->superclass = superclass;

  // Include the superclass in the total number of fields.
  subclass->numFields += superclass->numFields;

  // Inherit methods from its superclass.
  for (int i = 0; i < superclass->methods.count; i++)
  {
    wrenBindMethod(vm, subclass, i, superclass->methods.data[i]);
  }
}

ObjClass* wrenNewClass(WrenVM* vm, ObjClass* superclass, int numFields,
                       ObjString* name)
{
  wrenPushRoot(vm, (Obj*)name);

  // Create the metaclass.
  ObjString* metaclassName = wrenStringConcat(vm, name->value, name->length,
                                              " metaclass", -1);
  wrenPushRoot(vm, (Obj*)metaclassName);

  ObjClass* metaclass = wrenNewSingleClass(vm, 0, metaclassName);
  metaclass->obj.classObj = vm->classClass;

  wrenPopRoot(vm);

  // Make sure the metaclass isn't collected when we allocate the class.
  wrenPushRoot(vm, (Obj*)metaclass);

  // Metaclasses always inherit Class and do not parallel the non-metaclass
  // hierarchy.
  wrenBindSuperclass(vm, metaclass, vm->classClass);

  ObjClass* classObj = wrenNewSingleClass(vm, numFields, name);

  // Make sure the class isn't collected while the inherited methods are being
  // bound.
  wrenPushRoot(vm, (Obj*)classObj);

  classObj->obj.classObj = metaclass;
  wrenBindSuperclass(vm, classObj, superclass);

  wrenPopRoot(vm);
  wrenPopRoot(vm);
  wrenPopRoot(vm);

  return classObj;
}

void wrenBindMethod(WrenVM* vm, ObjClass* classObj, int symbol, Method method)
{
  // Make sure the buffer is big enough to reach the symbol's index.
  // TODO: Do a single grow instead of a loop.
  Method noMethod;
  noMethod.type = METHOD_NONE;
  while (symbol >= classObj->methods.count)
  {
    wrenMethodBufferWrite(vm, &classObj->methods, noMethod);
  }

  classObj->methods.data[symbol] = method;
}

ObjClosure* wrenNewClosure(WrenVM* vm, ObjFn* fn)
{
  ObjClosure* closure = ALLOCATE_FLEX(vm, ObjClosure,
                                      Upvalue*, fn->numUpvalues);
  initObj(vm, &closure->obj, OBJ_CLOSURE, vm->fnClass);

  closure->fn = fn;

  // Clear the upvalue array. We need to do this in case a GC is triggered
  // after the closure is created but before the upvalue array is populated.
  for (int i = 0; i < fn->numUpvalues; i++) closure->upvalues[i] = NULL;

  return closure;
}

ObjFiber* wrenNewFiber(WrenVM* vm, Obj* fn)
{
  ObjFiber* fiber = ALLOCATE(vm, ObjFiber);
  initObj(vm, &fiber->obj, OBJ_FIBER, vm->fiberClass);

  // Push the stack frame for the function.
  fiber->stackTop = fiber->stack;
  fiber->numFrames = 1;
  fiber->openUpvalues = NULL;
  fiber->caller = NULL;
  fiber->error = NULL;
  fiber->callerIsTrying = false;

  CallFrame* frame = &fiber->frames[0];
  frame->fn = fn;
  frame->stackStart = fiber->stack;
  if (fn->type == OBJ_FN)
  {
    frame->ip = ((ObjFn*)fn)->bytecode;
  }
  else
  {
    frame->ip = ((ObjClosure*)fn)->fn->bytecode;
  }

  return fiber;
}

ObjFn* wrenNewFunction(WrenVM* vm, Value* constants, int numConstants,
                       int numUpvalues, int arity,
                       uint8_t* bytecode, int bytecodeLength,
                       ObjString* debugSourcePath,
                       const char* debugName, int debugNameLength,
                       int* sourceLines)
{
  // Allocate these before the function in case they trigger a GC which would
  // free the function.
  Value* copiedConstants = NULL;
  if (numConstants > 0)
  {
    copiedConstants = ALLOCATE_ARRAY(vm, Value, numConstants);
    for (int i = 0; i < numConstants; i++)
    {
      copiedConstants[i] = constants[i];
    }
  }

  FnDebug* debug = ALLOCATE(vm, FnDebug);

  debug->sourcePath = debugSourcePath;

  // Copy the function's name.
  debug->name = ALLOCATE_ARRAY(vm, char, debugNameLength + 1);
  memcpy(debug->name, debugName, debugNameLength);
  debug->name[debugNameLength] = '\0';

  debug->sourceLines = sourceLines;

  ObjFn* fn = ALLOCATE(vm, ObjFn);
  initObj(vm, &fn->obj, OBJ_FN, vm->fnClass);

  // TODO: Should eventually copy this instead of taking ownership. When the
  // compiler grows this, its capacity will often exceed the actual used size.
  // Copying to an exact-sized buffer will save a bit of memory. I tried doing
  // this, but it made the "for" benchmark ~15% slower for some unknown reason.
  fn->bytecode = bytecode;
  fn->constants = copiedConstants;
  fn->numUpvalues = numUpvalues;
  fn->numConstants = numConstants;
  fn->arity = arity;
  fn->bytecodeLength = bytecodeLength;
  fn->debug = debug;

  return fn;
}

Value wrenNewInstance(WrenVM* vm, ObjClass* classObj)
{
  ObjInstance* instance = ALLOCATE_FLEX(vm, ObjInstance,
                                        Value, classObj->numFields);
  initObj(vm, &instance->obj, OBJ_INSTANCE, classObj);

  // Initialize fields to null.
  for (int i = 0; i < classObj->numFields; i++)
  {
    instance->fields[i] = NULL_VAL;
  }

  return OBJ_VAL(instance);
}

ObjList* wrenNewList(WrenVM* vm, int numElements)
{
  // Allocate this before the list object in case it triggers a GC which would
  // free the list.
  Value* elements = NULL;
  if (numElements > 0)
  {
    elements = ALLOCATE_ARRAY(vm, Value, numElements);
  }

  ObjList* list = ALLOCATE(vm, ObjList);
  initObj(vm, &list->obj, OBJ_LIST, vm->listClass);
  list->capacity = numElements;
  list->count = numElements;
  list->elements = elements;
  return list;
}

// Grows [list] if needed to ensure it can hold one more element.
static void ensureListCapacity(WrenVM* vm, ObjList* list)
{
  if (list->capacity >= list->count + 1) return;

  int capacity = list->capacity * GROW_FACTOR;
  if (capacity < MIN_CAPACITY) capacity = MIN_CAPACITY;

  list->capacity = capacity;
  list->elements = (Value*)wrenReallocate(vm, list->elements,
      list->capacity * sizeof(Value), capacity * sizeof(Value));
  // TODO: Handle allocation failure.
  list->capacity = capacity;
}

void wrenListAdd(WrenVM* vm, ObjList* list, Value value)
{
  if (IS_OBJ(value)) wrenPushRoot(vm, AS_OBJ(value));

  ensureListCapacity(vm, list);

  if (IS_OBJ(value)) wrenPopRoot(vm);

  list->elements[list->count++] = value;
}

void wrenListInsert(WrenVM* vm, ObjList* list, Value value, int index)
{
  if (IS_OBJ(value)) wrenPushRoot(vm, AS_OBJ(value));

  ensureListCapacity(vm, list);

  if (IS_OBJ(value)) wrenPopRoot(vm);

  // Shift items down.
  for (int i = list->count; i > index; i--)
  {
    list->elements[i] = list->elements[i - 1];
  }

  list->elements[index] = value;
  list->count++;
}

Value wrenListRemoveAt(WrenVM* vm, ObjList* list, int index)
{
  Value removed = list->elements[index];

  if (IS_OBJ(removed)) wrenPushRoot(vm, AS_OBJ(removed));

  // Shift items up.
  for (int i = index; i < list->count - 1; i++)
  {
    list->elements[i] = list->elements[i + 1];
  }

  // If we have too much excess capacity, shrink it.
  if (list->capacity / GROW_FACTOR >= list->count)
  {
    list->elements = (Value*)wrenReallocate(vm, list->elements,
        sizeof(Value) * list->capacity,
        sizeof(Value) * (list->capacity / GROW_FACTOR));
    list->capacity /= GROW_FACTOR;
  }

  if (IS_OBJ(removed)) wrenPopRoot(vm);

  list->count--;
  return removed;
}

ObjMap* wrenNewMap(WrenVM* vm)
{
  ObjMap* map = ALLOCATE(vm, ObjMap);
  initObj(vm, &map->obj, OBJ_MAP, vm->mapClass);
  map->capacity = 0;
  map->count = 0;
  map->entries = NULL;
  return map;
}

// Generates a hash code for [num].
static uint32_t hashNumber(double num)
{
  // Hash the raw bits of the value.
  DoubleBits data;
  data.num = num;
  return data.bits32[0] ^ data.bits32[1];
}

// Generates a hash code for [object].
static uint32_t hashObject(Obj* object)
{
  switch (object->type)
  {
    case OBJ_CLASS:
      // Classes just use their name.
      return hashObject((Obj*)((ObjClass*)object)->name);

    case OBJ_RANGE:
    {
      ObjRange* range = (ObjRange*)object;
      return hashNumber(range->from) ^ hashNumber(range->to);
    }

    case OBJ_STRING:
    {
      ObjString* string = (ObjString*)object;

      // FNV-1a hash. See: http://www.isthe.com/chongo/tech/comp/fnv/
      uint32_t hash = 2166136261u;

      // We want the contents of the string to affect the hash, but we also
      // want to ensure it runs in constant time. We also don't want to bias
      // towards the prefix or suffix of the string. So sample up to eight
      // characters spread throughout the string.
      // TODO: Tune this.
      uint32_t step = 1 + 7 / string->length;
      for (uint32_t i = 0; i < string->length; i += step)
      {
        hash ^= string->value[i];
        hash *= 16777619;
      }

      return hash;
    }

    default:
      ASSERT(false, "Only immutable objects can be hashed.");
      return 0;
  }
}

// Generates a hash code for [value], which must be one of the built-in
// immutable types: null, bool, class, num, range, or string.
static uint32_t hashValue(Value value)
{
  // TODO: We'll probably want to randomize this at some point.

#if WREN_NAN_TAGGING
  if (IS_NUM(value)) return hashNumber(AS_NUM(value));
  if (IS_OBJ(value)) return hashObject(AS_OBJ(value));

  switch (GET_TAG(value))
  {
    case TAG_FALSE: return HASH_FALSE;
    case TAG_NAN: return HASH_NAN;
    case TAG_NULL: return HASH_NULL;
    case TAG_TRUE: return HASH_TRUE;
    default:
      UNREACHABLE();
      return 0;
  }
#else
  switch (value.type)
  {
    case VAL_FALSE: return HASH_FALSE;
    case VAL_NULL: return HASH_NULL;
    case VAL_NUM: return hashNumber(AS_NUM(value));
    case VAL_TRUE: return HASH_TRUE;
    case VAL_OBJ: return hashObject(AS_OBJ(value));
    default:
      UNREACHABLE();
      return 0;
  }
#endif
}

// Inserts [key] and [value] in the array of [entries] with the given
// [capacity].
//
// Returns `true` if this is the first time [key] was added to the map.
static bool addEntry(MapEntry* entries, uint32_t capacity,
                     Value key, Value value)
{
  // Figure out where to insert it in the table. Use open addressing and
  // basic linear probing.
  uint32_t index = hashValue(key) % capacity;

  // We don't worry about an infinite loop here because ensureMapCapacity()
  // ensures there are open spaces in the table.
  while (true)
  {
    MapEntry* entry = &entries[index];

    // If we found an empty slot, the key is not in the table.
    if (IS_UNDEFINED(entry->key))
    {
      entry->key = key;
      entry->value = value;
      return true;
    }

    // If the key already exists, just replace the value.
    if (wrenValuesEqual(entry->key, key))
    {
      entry->value = value;
      return false;
    }

    // Try the next slot.
    index = (index + 1) % capacity;
  }
}

// Updates [map]'s entry array to [capacity].
static void resizeMap(WrenVM* vm, ObjMap* map, uint32_t capacity)
{
  // Create the new empty hash table.
  MapEntry* entries = ALLOCATE_ARRAY(vm, MapEntry, capacity);
  for (uint32_t i = 0; i < capacity; i++)
  {
    entries[i].key = UNDEFINED_VAL;
  }

  // Re-add the existing entries.
  if (map->capacity > 0)
  {
    for (uint32_t i = 0; i < map->capacity; i++)
    {
      MapEntry* entry = &map->entries[i];
      if (IS_UNDEFINED(entry->key)) continue;

      addEntry(entries, capacity, entry->key, entry->value);
    }
  }

  // Replace the array.
  wrenReallocate(vm, map->entries, 0, 0);
  map->entries = entries;
  map->capacity = capacity;
}

uint32_t wrenMapFind(ObjMap* map, Value key)
{
  // If there is no entry array (an empty map), we definitely won't find it.
  if (map->capacity == 0) return UINT32_MAX;

  // Figure out where to insert it in the table. Use open addressing and
  // basic linear probing.
  uint32_t index = hashValue(key) % map->capacity;

  // We don't worry about an infinite loop here because ensureMapCapacity()
  // ensures there are empty (i.e. UNDEFINED) spaces in the table.
  while (true)
  {
    MapEntry* entry = &map->entries[index];

    // If we found an empty slot, the key is not in the table.
    if (IS_UNDEFINED(entry->key)) return UINT32_MAX;

    // If the key matches, we found it.
    if (wrenValuesEqual(entry->key, key)) return index;

    // Try the next slot.
    index = (index + 1) % map->capacity;
  }
}

void wrenMapSet(WrenVM* vm, ObjMap* map, Value key, Value value)
{
  // If the map is getting too full, make room first.
  if (map->count + 1 > map->capacity * MAP_LOAD_PERCENT / 100)
  {
    // Figure out the new hash table size.
    uint32_t capacity = map->capacity * GROW_FACTOR;
    if (capacity < MIN_CAPACITY) capacity = MIN_CAPACITY;

    resizeMap(vm, map, capacity);
  }

  if (addEntry(map->entries, map->capacity, key, value))
  {
    // A new key was added.
    map->count++;
  }
}

void wrenMapClear(WrenVM* vm, ObjMap* map)
{
  wrenReallocate(vm, map->entries, 0, 0);
  map->entries = NULL;
  map->capacity = 0;
  map->count = 0;
}

Value wrenMapRemoveKey(WrenVM* vm, ObjMap* map, Value key)
{
  uint32_t index = wrenMapFind(map, key);
  if (index == UINT32_MAX) return NULL_VAL;

  // Remove the entry from the map.
  Value value = map->entries[index].value;
  map->entries[index].key = UNDEFINED_VAL;

  if (IS_OBJ(value)) wrenPushRoot(vm, AS_OBJ(value));

  map->count--;

  if (map->count == 0)
  {
    // Removed the last item, so free the array.
    wrenMapClear(vm, map);
    return value;
  }
  else if (map->count < map->capacity / GROW_FACTOR * MAP_LOAD_PERCENT / 100)
  {
    uint32_t capacity = map->capacity / GROW_FACTOR;
    if (capacity < MIN_CAPACITY) capacity = MIN_CAPACITY;

    // The map is getting empty, so shrink the entry array back down.
    // TODO: Should we do this less aggressively than we grow?
    resizeMap(vm, map, capacity);
  }
  else
  {
    // We've removed the entry, but we need to update any subsequent entries
    // that may have wanted to occupy its slot and got pushed down. Otherwise,
    // we won't be able to find them later.
    while (true)
    {
      index = (index + 1) % map->capacity;

      // When we hit an empty entry, we know we've handled every entry that may
      // have been affected by the removal.
      if (IS_UNDEFINED(map->entries[index].key)) break;

      // Re-add the entry so it gets dropped into the right slot.
      Value removedKey = map->entries[index].key;
      Value removedValue = map->entries[index].value;
      map->entries[index].key = UNDEFINED_VAL;

      addEntry(map->entries, map->capacity, removedKey, removedValue);
    }
  }

  if (IS_OBJ(value)) wrenPopRoot(vm);
  return value;
}

Value wrenNewRange(WrenVM* vm, double from, double to, bool isInclusive)
{
  ObjRange* range = ALLOCATE(vm, ObjRange);
  initObj(vm, &range->obj, OBJ_RANGE, vm->rangeClass);
  range->from = from;
  range->to = to;
  range->isInclusive = isInclusive;

  return OBJ_VAL(range);
}

Value wrenNewString(WrenVM* vm, const char* text, size_t length)
{
  // Allow NULL if the string is empty since byte buffers don't allocate any
  // characters for a zero-length string.
  ASSERT(length == 0 || text != NULL, "Unexpected NULL string.");

  // TODO: Don't allocate a heap string at all for zero-length strings.
  ObjString* string = AS_STRING(wrenNewUninitializedString(vm, length));

  // Copy the string (if given one).
  if (length > 0) memcpy(string->value, text, length);

  string->value[length] = '\0';

  return OBJ_VAL(string);
}

Value wrenNewUninitializedString(WrenVM* vm, size_t length)
{
  ObjString* string = ALLOCATE_FLEX(vm, ObjString, char, length + 1);
  initObj(vm, &string->obj, OBJ_STRING, vm->stringClass);
  string->length = (int)length;

  return OBJ_VAL(string);
}

ObjString* wrenStringConcat(WrenVM* vm, const char* left, int leftLength,
                            const char* right, int rightLength)
{
  if (leftLength == -1) leftLength = (int)strlen(left);
  if (rightLength == -1) rightLength = (int)strlen(right);

  Value value = wrenNewUninitializedString(vm, leftLength + rightLength);
  ObjString* string = AS_STRING(value);
  memcpy(string->value, left, leftLength);
  memcpy(string->value + leftLength, right, rightLength);
  string->value[leftLength + rightLength] = '\0';

  return string;
}

Value wrenStringCodePointAt(WrenVM* vm, ObjString* string, int index)
{
  ASSERT(index >= 0, "Index out of bounds.");
  ASSERT(index < string->length, "Index out of bounds.");

  char first = string->value[index];

  // The first byte's high bits tell us how many bytes are in the UTF-8
  // sequence. If the byte starts with 10xxxxx, it's the middle of a UTF-8
  // sequence, so return an empty string.
  int numBytes;
  if      ((first & 0xc0) == 0x80) numBytes = 0;
  else if ((first & 0xf8) == 0xf0) numBytes = 4;
  else if ((first & 0xf0) == 0xe0) numBytes = 3;
  else if ((first & 0xe0) == 0xc0) numBytes = 2;
  else numBytes = 1;

  Value value = wrenNewUninitializedString(vm, numBytes);
  ObjString* result = AS_STRING(value);
  memcpy(result->value, string->value + index, numBytes);
  result->value[numBytes] = '\0';
  return value;
}

// Implementing Boyer-Moore-Horspool string matching algorithm.
uint32_t wrenStringFind(WrenVM* vm, ObjString* haystack, ObjString* needle)
{
  uint32_t length, last_index, index;
  uint32_t shift[256]; // Full 8-bit alphabet
  char last_char;
  // We need to convert the 'char' value to 'unsigned char' in order not
  // to get negative indexes. We use a union as an elegant alternative to
  // explicit coercion (whose behaviour is not sure across compilers)
  union {
    char c;
    unsigned char uc;
  } value;

  // If the needle is longer than the haystack it won't be found.
  if (needle->length > haystack->length)
  {
    return haystack->length;
  }

  // Precalculate shifts.
  last_index = needle->length - 1;

  for (index = 0; index < 256; index++)
  {
    shift[index] = needle->length;
  }
  for (index = 0; index < last_index; index++)
  {
    value.c = needle->value[index];
    shift[value.uc] = last_index - index;
  }

  // Search, left to right.
  last_char = needle->value[last_index];

  length = haystack->length - needle->length;

  for (index = 0; index <= length; )
  {
    value.c = haystack->value[index + last_index];
    if ((last_char == value.c) &&
      memcmp(haystack->value + index, needle->value, last_index) == 0)
    {
      return index;
    }
    index += shift[value.uc]; // Convert, same as above.
  }

  // Not found.
  return haystack->length;
}

Upvalue* wrenNewUpvalue(WrenVM* vm, Value* value)
{
  Upvalue* upvalue = ALLOCATE(vm, Upvalue);

  // Upvalues are never used as first-class objects, so don't need a class.
  initObj(vm, &upvalue->obj, OBJ_UPVALUE, NULL);

  upvalue->value = value;
  upvalue->closed = NULL_VAL;
  upvalue->next = NULL;
  return upvalue;
}

// Sets the mark flag on [obj]. Returns true if it was already set so that we
// can avoid recursing into already-processed objects. That ensures we don't
// crash on an object cycle.
static bool setMarkedFlag(Obj* obj)
{
  if (obj->marked) return true;
  obj->marked = true;
  return false;
}

static void markString(WrenVM* vm, ObjString* string)
{
  if (setMarkedFlag(&string->obj)) return;

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjString) + string->length + 1;
}

static void markClass(WrenVM* vm, ObjClass* classObj)
{
  if (setMarkedFlag(&classObj->obj)) return;

  // The metaclass.
  if (classObj->obj.classObj != NULL) markClass(vm, classObj->obj.classObj);

  // The superclass.
  if (classObj->superclass != NULL) markClass(vm, classObj->superclass);

  // Method function objects.
  for (int i = 0; i < classObj->methods.count; i++)
  {
    if (classObj->methods.data[i].type == METHOD_BLOCK)
    {
      wrenMarkObj(vm, classObj->methods.data[i].fn.obj);
    }
  }

  if (classObj->name != NULL) markString(vm, classObj->name);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjClass);
  vm->bytesAllocated += classObj->methods.capacity * sizeof(Method);
}

static void markFn(WrenVM* vm, ObjFn* fn)
{
  if (setMarkedFlag(&fn->obj)) return;

  // Mark the constants.
  for (int i = 0; i < fn->numConstants; i++)
  {
    wrenMarkValue(vm, fn->constants[i]);
  }

  wrenMarkObj(vm, (Obj*)fn->debug->sourcePath);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjFn);
  vm->bytesAllocated += sizeof(uint8_t) * fn->bytecodeLength;
  vm->bytesAllocated += sizeof(Value) * fn->numConstants;

  // The debug line number buffer.
  vm->bytesAllocated += sizeof(int) * fn->bytecodeLength;

  // TODO: What about the function name?
}

static void markInstance(WrenVM* vm, ObjInstance* instance)
{
  if (setMarkedFlag(&instance->obj)) return;

  markClass(vm, instance->obj.classObj);

  // Mark the fields.
  for (int i = 0; i < instance->obj.classObj->numFields; i++)
  {
    wrenMarkValue(vm, instance->fields[i]);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjInstance);
  vm->bytesAllocated += sizeof(Value) * instance->obj.classObj->numFields;
}

static void markList(WrenVM* vm, ObjList* list)
{
  if (setMarkedFlag(&list->obj)) return;

  // Mark the elements.
  Value* elements = list->elements;
  for (int i = 0; i < list->count; i++)
  {
    wrenMarkValue(vm, elements[i]);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjList);
  vm->bytesAllocated += sizeof(Value) * list->capacity;
}

static void markMap(WrenVM* vm, ObjMap* map)
{
  if (setMarkedFlag(&map->obj)) return;

  // Mark the entries.
  for (int i = 0; i < map->capacity; i++)
  {
    MapEntry* entry = &map->entries[i];
    if (IS_UNDEFINED(entry->key)) continue;

    wrenMarkValue(vm, entry->key);
    wrenMarkValue(vm, entry->value);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjMap);
  vm->bytesAllocated += sizeof(MapEntry) * map->capacity;
}

static void markUpvalue(WrenVM* vm, Upvalue* upvalue)
{
  // This can happen if a GC is triggered in the middle of initializing the
  // closure.
  if (upvalue == NULL) return;

  if (setMarkedFlag(&upvalue->obj)) return;

  // Mark the closed-over object (in case it is closed).
  wrenMarkValue(vm, upvalue->closed);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(Upvalue);
}

static void markFiber(WrenVM* vm, ObjFiber* fiber)
{
  if (setMarkedFlag(&fiber->obj)) return;

  // Stack functions.
  for (int i = 0; i < fiber->numFrames; i++)
  {
    wrenMarkObj(vm, fiber->frames[i].fn);
  }

  // Stack variables.
  for (Value* slot = fiber->stack; slot < fiber->stackTop; slot++)
  {
    wrenMarkValue(vm, *slot);
  }

  // Open upvalues.
  Upvalue* upvalue = fiber->openUpvalues;
  while (upvalue != NULL)
  {
    markUpvalue(vm, upvalue);
    upvalue = upvalue->next;
  }

  // The caller.
  if (fiber->caller != NULL) markFiber(vm, fiber->caller);

  if (fiber->error != NULL) markString(vm, fiber->error);

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjFiber);
  // TODO: Count size of error message buffer.
}

static void markClosure(WrenVM* vm, ObjClosure* closure)
{
  if (setMarkedFlag(&closure->obj)) return;

  // Mark the function.
  markFn(vm, closure->fn);

  // Mark the upvalues.
  for (int i = 0; i < closure->fn->numUpvalues; i++)
  {
    Upvalue* upvalue = closure->upvalues[i];
    markUpvalue(vm, upvalue);
  }

  // Keep track of how much memory is still in use.
  vm->bytesAllocated += sizeof(ObjClosure);
  vm->bytesAllocated += sizeof(Upvalue*) * closure->fn->numUpvalues;
}

void wrenMarkObj(WrenVM* vm, Obj* obj)
{
#if WREN_DEBUG_TRACE_MEMORY
  static int indent = 0;
  indent++;
  for (int i = 0; i < indent; i++) printf("  ");
  printf("mark ");
  wrenPrintValue(OBJ_VAL(obj));
  printf(" @ %p\n", obj);
#endif

  // Traverse the object's fields.
  switch (obj->type)
  {
    case OBJ_CLASS:    markClass(   vm, (ObjClass*)   obj); break;
    case OBJ_CLOSURE:  markClosure( vm, (ObjClosure*) obj); break;
    case OBJ_FIBER:    markFiber(   vm, (ObjFiber*)   obj); break;
    case OBJ_FN:       markFn(      vm, (ObjFn*)      obj); break;
    case OBJ_INSTANCE: markInstance(vm, (ObjInstance*)obj); break;
    case OBJ_LIST:     markList(    vm, (ObjList*)    obj); break;
    case OBJ_MAP:      markMap(     vm, (ObjMap*)     obj); break;
    case OBJ_RANGE:    setMarkedFlag(obj); break;
    case OBJ_STRING:   markString(  vm, (ObjString*)  obj); break;
    case OBJ_UPVALUE:  markUpvalue( vm, (Upvalue*)    obj); break;
  }

#if WREN_DEBUG_TRACE_MEMORY
  indent--;
#endif
}

void wrenMarkValue(WrenVM* vm, Value value)
{
  if (!IS_OBJ(value)) return;
  wrenMarkObj(vm, AS_OBJ(value));
}

void wrenFreeObj(WrenVM* vm, Obj* obj)
{
#if WREN_DEBUG_TRACE_MEMORY
  printf("free ");
  wrenPrintValue(OBJ_VAL(obj));
  printf(" @ %p\n", obj);
#endif

  switch (obj->type)
  {
    case OBJ_CLASS:
      wrenMethodBufferClear(vm, &((ObjClass*)obj)->methods);
      break;

    case OBJ_FN:
    {
      ObjFn* fn = (ObjFn*)obj;
      wrenReallocate(vm, fn->constants, 0, 0);
      wrenReallocate(vm, fn->bytecode, 0, 0);
      wrenReallocate(vm, fn->debug->name, 0, 0);
      wrenReallocate(vm, fn->debug->sourceLines, 0, 0);
      wrenReallocate(vm, fn->debug, 0, 0);
      break;
    }

    case OBJ_LIST:
      wrenReallocate(vm, ((ObjList*)obj)->elements, 0, 0);
      break;

    case OBJ_MAP:
      wrenReallocate(vm, ((ObjMap*)obj)->entries, 0, 0);
      break;

    case OBJ_STRING:
    case OBJ_CLOSURE:
    case OBJ_FIBER:
    case OBJ_INSTANCE:
    case OBJ_RANGE:
    case OBJ_UPVALUE:
      break;
  }

  wrenReallocate(vm, obj, 0, 0);
}

ObjClass* wrenGetClass(WrenVM* vm, Value value)
{
  return wrenGetClassInline(vm, value);
}

bool wrenValuesEqual(Value a, Value b)
{
  if (wrenValuesSame(a, b)) return true;

  // If we get here, it's only possible for two heap-allocated immutable objects
  // to be equal.
  if (!IS_OBJ(a) || !IS_OBJ(b)) return false;

  Obj* aObj = AS_OBJ(a);
  Obj* bObj = AS_OBJ(b);

  // Must be the same type.
  if (aObj->type != bObj->type) return false;

  switch (aObj->type)
  {
    case OBJ_RANGE:
    {
      ObjRange* aRange = (ObjRange*)aObj;
      ObjRange* bRange = (ObjRange*)bObj;
      return aRange->from == bRange->from &&
             aRange->to == bRange->to &&
             aRange->isInclusive == bRange->isInclusive;
    }

    case OBJ_STRING:
    {
      ObjString* aString = (ObjString*)aObj;
      ObjString* bString = (ObjString*)bObj;
      return aString->length == bString->length &&
             memcmp(aString->value, bString->value, aString->length) == 0;
    }

    default:
      // All other types are only equal if they are same, which they aren't if
      // we get here.
      return false;
  }
}

static void printObject(Obj* obj)
{
  switch (obj->type)
  {
    case OBJ_CLASS: printf("[class %p]", obj); break;
    case OBJ_CLOSURE: printf("[closure %p]", obj); break;
    case OBJ_FIBER: printf("[fiber %p]", obj); break;
    case OBJ_FN: printf("[fn %p]", obj); break;
    case OBJ_INSTANCE: printf("[instance %p]", obj); break;
    case OBJ_LIST: printf("[list %p]", obj); break;
    case OBJ_MAP: printf("[map %p]", obj); break;
    case OBJ_RANGE: printf("[fn %p]", obj); break;
    case OBJ_STRING: printf("%s", ((ObjString*)obj)->value); break;
    case OBJ_UPVALUE: printf("[upvalue %p]", obj); break;
    default: printf("[unknown object]"); break;
  }
}

void wrenPrintValue(Value value)
{
  #if WREN_NAN_TAGGING
  if (IS_NUM(value))
  {
    printf("%.14g", AS_NUM(value));
  }
  else if (IS_OBJ(value))
  {
    printObject(AS_OBJ(value));
  }
  else
  {
    switch (GET_TAG(value))
    {
      case TAG_FALSE: printf("false"); break;
      case TAG_NAN: printf("NaN"); break;
      case TAG_NULL: printf("null"); break;
      case TAG_TRUE: printf("true"); break;
      case TAG_UNDEFINED: UNREACHABLE();
    }
  }
  #else
  switch (value.type)
  {
    case VAL_FALSE: printf("false"); break;
    case VAL_NULL: printf("null"); break;
    case VAL_NUM: printf("%.14g", AS_NUM(value)); break;
    case VAL_TRUE: printf("true"); break;
    case VAL_OBJ: printObject(AS_OBJ(value)); break;
    case VAL_UNDEFINED: UNREACHABLE();
  }
  #endif
}
