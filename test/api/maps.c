#include <string.h>

#include "maps.h"

static void newMap(WrenVM* vm)
{
  wrenSetSlotNewMap(vm, 0);
}

static void insert(WrenVM* vm)
{
  wrenSetSlotNewMap(vm, 0);
  
  wrenEnsureSlots(vm, 3);

  // Insert String
  wrenSetSlotString(vm, 1, "England");
  wrenSetSlotString(vm, 2, "London");
  wrenInsertInMap(vm, 0, 1, 2);

  // Insert Double
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 42.0);
  wrenInsertInMap(vm, 0, 1, 2);

  // Insert Boolean
  wrenSetSlotBool(vm, 1, false);
  wrenSetSlotBool(vm, 2, true);
  wrenInsertInMap(vm, 0, 1, 2);

  // Insert Null
  wrenSetSlotNull(vm, 1);
  wrenSetSlotNull(vm, 2);
  wrenInsertInMap(vm, 0, 1, 2);

  // Insert List
  wrenSetSlotString(vm, 1, "Empty");
  wrenSetSlotNewList(vm, 2);
  wrenInsertInMap(vm, 0, 1, 2);
}

WrenForeignMethodFn mapsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Maps.newMap()") == 0) return newMap;
  if (strcmp(signature, "static Maps.insert()") == 0) return insert;

  return NULL;
}
