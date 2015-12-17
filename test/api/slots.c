#include <stdio.h>
#include <string.h>

#include "slots.h"

static void noSet(WrenVM* vm)
{
  // Do nothing.
}

static void getSlots(WrenVM* vm)
{
  bool result = true;
  if (wrenGetSlotBool(vm, 1) != true) result = false;
  // TODO: Test wrenGetSlotForeign().
  
  int length;
  const char* bytes = wrenGetSlotBytes(vm, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;

  if (wrenGetSlotDouble(vm, 3) != 12.34) result = false;
  if (strcmp(wrenGetSlotString(vm, 4), "str") != 0) result = false;
  
  WrenValue* value = wrenGetSlotValue(vm, 5);

  if (result)
  {
    // Otherwise, return the value so we can tell if we captured it correctly.
    wrenSetSlotValue(vm, 0, value);
    wrenReleaseValue(vm, value);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(vm, 0, false);
  }
}

static void setSlots(WrenVM* vm)
{
  WrenValue* value = wrenGetSlotValue(vm, 1);
  
  wrenSetSlotBool(vm, 1, true);
  wrenSetSlotBytes(vm, 2, "by\0te", 5);
  wrenSetSlotDouble(vm, 3, 12.34);
  wrenSetSlotString(vm, 4, "str");
  
  // TODO: wrenSetSlotNull().
  
  // Read the slots back to make sure they were set correctly.
  
  bool result = true;
  if (wrenGetSlotBool(vm, 1) != true) result = false;
  
  int length;
  const char* bytes = wrenGetSlotBytes(vm, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;
  
  if (wrenGetSlotDouble(vm, 3) != 12.34) result = false;
  if (strcmp(wrenGetSlotString(vm, 4), "str") != 0) result = false;

  if (result)
  {
    // Move the value into the return position.
    wrenSetSlotValue(vm, 0, value);
    wrenReleaseValue(vm, value);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(vm, 0, false);
  }
}

static void ensure(WrenVM* vm)
{
  int before = wrenGetSlotCount(vm);
  
  wrenEnsureSlots(vm, 20);
  
  int after = wrenGetSlotCount(vm);
  
  // Use the slots to make sure they're available.
  for (int i = 0; i < 20; i++)
  {
    wrenSetSlotDouble(vm, i, i);
  }
  
  int sum = 0;

  for (int i = 0; i < 20; i++)
  {
    sum += (int)wrenGetSlotDouble(vm, i);
  }
  
  char result[100];
  sprintf(result, "%d -> %d (%d)", before, after, sum);
  wrenSetSlotString(vm, 0, result);
}

WrenForeignMethodFn slotsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Slots.noSet") == 0) return noSet;
  if (strcmp(signature, "static Slots.getSlots(_,_,_,_,_)") == 0) return getSlots;
  if (strcmp(signature, "static Slots.setSlots(_,_,_,_)") == 0) return setSlots;
  if (strcmp(signature, "static Slots.ensure()") == 0) return ensure;

  return NULL;
}
