#include <string.h>

#include "lists.h"

static void newList(WrenVM* vm)
{
  wrenSetSlotNewList(vm, 0);
}

// Helper function to store a double in a slot then insert it into the list at
// slot zero.
static void insertNumber(WrenVM* vm, int index, double value)
{
  wrenSetSlotDouble(vm, 1, value);
  wrenInsertInList(vm, 0, index, 1);
}

static void insert(WrenVM* vm)
{
  wrenSetSlotNewList(vm, 0);
  
  wrenEnsureSlots(vm, 2);
  
  // Appending.
  insertNumber(vm, 0, 1.0);
  insertNumber(vm, 1, 2.0);
  insertNumber(vm, 2, 3.0);
  
  // Inserting.
  insertNumber(vm, 0, 4.0);
  insertNumber(vm, 1, 5.0);
  insertNumber(vm, 2, 6.0);
  
  // Negative indexes.
  insertNumber(vm, -1, 7.0);
  insertNumber(vm, -2, 8.0);
  insertNumber(vm, -3, 9.0);
}

WrenForeignMethodFn listsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Lists.newList()") == 0) return newList;
  if (strcmp(signature, "static Lists.insert()") == 0) return insert;

  return NULL;
}
