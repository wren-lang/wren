#include <string.h>

#include "lists.h"

static void newList(WrenFiber* fiber)
{
  wrenSetSlotNewList(fiber, 0);
}

// Helper function to store a double in a slot then insert it into the list at
// slot zero.
static void insertNumber(WrenFiber* fiber, int index, double value)
{
  wrenSetSlotDouble(fiber, 1, value);
  wrenInsertInList(fiber, 0, index, 1);
}

static void insert(WrenFiber* fiber)
{
  wrenSetSlotNewList(fiber, 0);
  
  wrenSetSlotCount(fiber, 2);
  
  // Appending.
  insertNumber(fiber, 0, 1.0);
  insertNumber(fiber, 1, 2.0);
  insertNumber(fiber, 2, 3.0);
  
  // Inserting.
  insertNumber(fiber, 0, 4.0);
  insertNumber(fiber, 1, 5.0);
  insertNumber(fiber, 2, 6.0);
  
  // Negative indexes.
  insertNumber(fiber, -1, 7.0);
  insertNumber(fiber, -2, 8.0);
  insertNumber(fiber, -3, 9.0);
}

WrenForeignMethodFn listsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Lists.newList()") == 0) return newList;
  if (strcmp(signature, "static Lists.insert()") == 0) return insert;

  return NULL;
}
