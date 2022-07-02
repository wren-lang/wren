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

  int length;
  const char* bytes = wrenGetSlotBytes(vm, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;

  if (wrenGetSlotDouble(vm, 3) != 1.5) result = false;
  if (strcmp(wrenGetSlotString(vm, 4), "str") != 0) result = false;

  WrenHandle* handle = wrenGetSlotHandle(vm, 5);

  if (result)
  {
    // Otherwise, return the value so we can tell if we captured it correctly.
    wrenSetSlotHandle(vm, 0, handle);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(vm, 0, false);
  }

  wrenReleaseHandle(vm, handle);
}

static void setSlots(WrenVM* vm)
{
  WrenHandle* handle = wrenGetSlotHandle(vm, 1);

  wrenSetSlotBool(vm, 1, true);
  wrenSetSlotBytes(vm, 2, "by\0te", 5);
  wrenSetSlotDouble(vm, 3, 1.5);
  wrenSetSlotString(vm, 4, "str");
  wrenSetSlotNull(vm, 5);

  // Read the slots back to make sure they were set correctly.

  bool result = true;
  if (wrenGetSlotBool(vm, 1) != true) result = false;

  int length;
  const char* bytes = wrenGetSlotBytes(vm, 2, &length);
  if (length != 5) result = false;
  if (memcmp(bytes, "by\0te", length) != 0) result = false;

  if (wrenGetSlotDouble(vm, 3) != 1.5) result = false;
  if (strcmp(wrenGetSlotString(vm, 4), "str") != 0) result = false;

  if (wrenGetSlotType(vm, 5) != WREN_TYPE_NULL) result = false;

  if (result)
  {
    // Move the value into the return position.
    wrenSetSlotHandle(vm, 0, handle);
  }
  else
  {
    // If anything failed, return false.
    wrenSetSlotBool(vm, 0, false);
  }

  wrenReleaseHandle(vm, handle);
}

static void slotTypes(WrenVM* vm)
{
  bool result =
      wrenGetSlotType(vm, 1) == WREN_TYPE_BOOL &&
      wrenGetSlotType(vm, 2) == WREN_TYPE_FOREIGN &&
      wrenGetSlotType(vm, 3) == WREN_TYPE_LIST &&
      wrenGetSlotType(vm, 4) == WREN_TYPE_MAP &&
      wrenGetSlotType(vm, 5) == WREN_TYPE_NULL &&
      wrenGetSlotType(vm, 6) == WREN_TYPE_NUM &&
      wrenGetSlotType(vm, 7) == WREN_TYPE_STRING &&
      wrenGetSlotType(vm, 8) == WREN_TYPE_UNKNOWN;

  wrenSetSlotBool(vm, 0, result);
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

static void ensureOutsideForeign(WrenVM* vm)
{
  // To test the behavior outside of a foreign method (which we're currently
  // in), create a new separate VM.
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  WrenVM* otherVM = wrenNewVM(&config);

  int before = wrenGetSlotCount(otherVM);

  wrenEnsureSlots(otherVM, 20);

  int after = wrenGetSlotCount(otherVM);

  // Use the slots to make sure they're available.
  for (int i = 0; i < 20; i++)
  {
    wrenSetSlotDouble(otherVM, i, i);
  }

  int sum = 0;

  for (int i = 0; i < 20; i++)
  {
    sum += (int)wrenGetSlotDouble(otherVM, i);
  }

  wrenFreeVM(otherVM);

  char result[100];
  sprintf(result, "%d -> %d (%d)", before, after, sum);
  wrenSetSlotString(vm, 0, result);
}

static void foreignClassAllocate(WrenVM* vm)
{
  wrenSetSlotNewForeign(vm, 0, 0, 4);
}

static void getListCount(WrenVM* vm)
{
  wrenSetSlotDouble(vm, 0, wrenGetListCount(vm, 1));
}

static void getListElement(WrenVM* vm)
{
  int index = (int)wrenGetSlotDouble(vm, 2);
  wrenGetListElement(vm, 1, index, 0);
}

static void getMapValue(WrenVM* vm)
{
  wrenGetMapValue(vm, 1, 2, 0);
}

static void getSlotClass(WrenVM* vm)
{
  wrenGetSlotClass(vm, 1, 0);
}

static void getSlotClassName(WrenVM* vm)
{
  const char* name = wrenGetSlotClassName(vm, 1);

  if (name == NULL)
  {
    wrenSetSlotNull(vm, 0);
  }
  else
  {
    wrenSetSlotString(vm, 0, name);
  }
}

static void isParameterForeignType(WrenVM* vm)
{
    // First, get whatever variable is named "ForeignType" in the slots module.
    wrenGetVariable(vm, "./test/api/slots", "ForeignType", 0);

    // Then, retrieve the class of the variable.
    wrenGetSlotClass(vm, 0, 0);

    // Then, retrieve the class of the parameter.
    wrenGetSlotClass(vm, 1, 1);

    // Finally, check that both are "the same".
    wrenSetSlotBool(vm, 0, wrenIsSameClass(vm, 0, 1));
}

static void isParameterForeignTypeByName(WrenVM* vm)
{
    // First, retrieve the class of the parameter.
    wrenGetSlotClass(vm, 1, 1);

    // Finally, check that the class is named "ForeignType".
    //
    // Please note that this is definitely what you should not do to ensure
    // you've been given the right parameter class, as the "ForeignType" here
    // only validates the name and not its origin.
    // From what we know, the parameter we've been given might come from any
    // module, "./test/api/slots" or anything else.
    const char* name = wrenGetSlotClassName(vm, 1);
    wrenSetSlotBool(vm, 0, strcmp(name, "ForeignType") == 0);
}

WrenForeignMethodFn slotsBindMethod(const char* signature)
{
  if (strcmp(signature, "static Slots.noSet") == 0) return noSet;
  if (strcmp(signature, "static Slots.getSlots(_,_,_,_,_)") == 0) return getSlots;
  if (strcmp(signature, "static Slots.setSlots(_,_,_,_,_)") == 0) return setSlots;
  if (strcmp(signature, "static Slots.slotTypes(_,_,_,_,_,_,_,_)") == 0) return slotTypes;
  if (strcmp(signature, "static Slots.ensure()") == 0) return ensure;
  if (strcmp(signature, "static Slots.ensureOutsideForeign()") == 0) return ensureOutsideForeign;
  if (strcmp(signature, "static Slots.getListCount(_)") == 0) return getListCount;
  if (strcmp(signature, "static Slots.getListElement(_,_)") == 0) return getListElement;
  if (strcmp(signature, "static Slots.getMapValue(_,_)") == 0) return getMapValue;
  if (strcmp(signature, "static Slots.getSlotClass(_)") == 0) return getSlotClass;
  if (strcmp(signature, "static Slots.getSlotClassName(_)") == 0) return getSlotClassName;
  if (strcmp(signature, "static Slots.isParameterForeignType(_)") == 0) return isParameterForeignType;
  if (strcmp(signature, "static Slots.isParameterForeignTypeByName(_)") == 0) return isParameterForeignTypeByName;

  return NULL;
}

void slotsBindClass(const char* className, WrenForeignClassMethods* methods)
{
  methods->allocate = foreignClassAllocate;
}
