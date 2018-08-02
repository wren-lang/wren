#include <stdio.h>
#include <string.h>

#include "slots.h"

static void noSet(WrenFiber* fiber)
{
  // Do nothing.
}

static void getSlots(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  bool result = true;
  if (wrenGetSlotBool(fiber, 1) != true) result = false;
  
  int length;
  const char* bytes = wrenGetSlotBytes(fiber, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;

  if (wrenGetSlotDouble(fiber, 3) != 1.5) result = false;
  if (strcmp(wrenGetSlotString(fiber, 4), "str") != 0) result = false;
  
  WrenHandle* handle = wrenGetSlotHandle(fiber, 5);

  if (result)
  {
    // Otherwise, return the value so we can tell if we captured it correctly.
    wrenSetSlotHandle(fiber, 0, handle);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(fiber, 0, false);
  }

  wrenReleaseHandle(vm, handle);
}

static void setSlots(WrenFiber* fiber)
{
  WrenVM* vm = wrenGetVM(fiber);
  WrenHandle* handle = wrenGetSlotHandle(fiber, 1);
  
  wrenSetSlotBool(fiber, 1, true);
  wrenSetSlotBytes(fiber, 2, "by\0te", 5);
  wrenSetSlotDouble(fiber, 3, 1.5);
  wrenSetSlotString(fiber, 4, "str");
  wrenSetSlotNull(fiber, 5);
  
  // Read the slots back to make sure they were set correctly.
  
  bool result = true;
  if (wrenGetSlotBool(fiber, 1) != true) result = false;
  
  int length;
  const char* bytes = wrenGetSlotBytes(fiber, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;

  if (wrenGetSlotDouble(fiber, 3) != 1.5) result = false;
  if (strcmp(wrenGetSlotString(fiber, 4), "str") != 0) result = false;

  if (wrenGetSlotType(fiber, 5) != WREN_TYPE_NULL) result = false;
  
  if (result)
  {
    // Move the value into the return position.
    wrenSetSlotHandle(fiber, 0, handle);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(fiber, 0, false);
  }

  wrenReleaseHandle(vm, handle);
}

static void slotTypes(WrenFiber* fiber)
{
  bool result =
      wrenGetSlotType(fiber, 1) == WREN_TYPE_BOOL &&
      wrenGetSlotType(fiber, 2) == WREN_TYPE_FOREIGN &&
      wrenGetSlotType(fiber, 3) == WREN_TYPE_LIST &&
      wrenGetSlotType(fiber, 4) == WREN_TYPE_NULL &&
      wrenGetSlotType(fiber, 5) == WREN_TYPE_NUM &&
      wrenGetSlotType(fiber, 6) == WREN_TYPE_STRING &&
      wrenGetSlotType(fiber, 7) == WREN_TYPE_UNKNOWN;
  
  wrenSetSlotBool(fiber, 0, result);
}

static void ensure(WrenFiber* fiber)
{
  int before = wrenGetSlotCount(fiber);
  
  wrenSetSlotCount(fiber, 20);
  
  int after = wrenGetSlotCount(fiber);
  
  // Use the slots to make sure they're available.
  for (int i = 0; i < 20; i++)
  {
    wrenSetSlotDouble(fiber, i, i);
  }
  
  int sum = 0;

  for (int i = 0; i < 20; i++)
  {
    sum += (int)wrenGetSlotDouble(fiber, i);
  }

  char result[100];
  sprintf(result, "%d -> %d (%d)", before, after, sum);
  wrenSetSlotString(fiber, 0, result);
}

static void ensureOutsideForeign(WrenFiber* fiber)
{
  // To test the behavior outside of a foreign method (which we're currently
  // in), create a new separate VM.
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  WrenVM* otherVM = wrenNewVM(&config);
  WrenFiber* otherFiber = wrenGetCurrentFiber(otherVM);

  int before = wrenGetSlotCount(otherFiber);

  wrenSetSlotCount(otherFiber, 20);

  int after = wrenGetSlotCount(otherFiber);

  // Use the slots to make sure they're available.
  for (int i = 0; i < 20; i++)
  {
    wrenSetSlotDouble(otherFiber, i, i);
  }

  int sum = 0;

  for (int i = 0; i < 20; i++)
  {
    sum += (int)wrenGetSlotDouble(otherFiber, i);
  }

  wrenFreeVM(otherVM);

  char result[100];
  sprintf(result, "%d -> %d (%d)", before, after, sum);
  wrenSetSlotString(fiber, 0, result);
}

static void foreignClassAllocate(WrenFiber* fiber)
{
  wrenSetSlotNewForeign(fiber, 0, 0, 4);
}

static void getListCount(WrenFiber* fiber)
{
  wrenSetSlotDouble(fiber, 0, wrenGetListCount(fiber, 1));
}

static void getListElement(WrenFiber* fiber)
{
  int index = (int)wrenGetSlotDouble(fiber, 2);
  wrenGetListElement(fiber, 1, index, 0);
}

WrenForeignMethodFn slotsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Slots.noSet") == 0) return noSet;
  if (strcmp(signature, "static Slots.getSlots(_,_,_,_,_)") == 0) return getSlots;
  if (strcmp(signature, "static Slots.setSlots(_,_,_,_,_)") == 0) return setSlots;
  if (strcmp(signature, "static Slots.slotTypes(_,_,_,_,_,_,_)") == 0) return slotTypes;
  if (strcmp(signature, "static Slots.ensure()") == 0) return ensure;
  if (strcmp(signature, "static Slots.ensureOutsideForeign()") == 0) return ensureOutsideForeign;
  if (strcmp(signature, "static Slots.getListCount(_)") == 0) return getListCount;
  if (strcmp(signature, "static Slots.getListElement(_,_)") == 0) return getListElement;

  return NULL;
}

void slotsBindClass(const char* className, WrenForeignClassMethods* methods)
{
  methods->allocate = foreignClassAllocate;
}
