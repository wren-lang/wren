#include <stdio.h>
#include <string.h>

#include "foreign_constructor.h"

static void adderAllocate(WrenVM* vm)
{
  double* total = (double*)wrenSetSlotNewForeign(vm, 0, 0, sizeof(double));

  int count = wrenGetListCount(vm, 1);

  int slots = wrenGetSlotCount(vm);
  const int aux_slot_id = slots;
  wrenEnsureSlots(vm, aux_slot_id + 1);

  *total = 0;
  for (int i = 0; i < count; ++i) {
      wrenGetListElement(vm, 1, i, aux_slot_id);

      *total += wrenGetSlotDouble(vm, aux_slot_id);
  }
}

static void adderTotal(WrenVM* vm)
{
  double total = *(double*)wrenGetSlotForeign(vm, 0);
  wrenSetSlotDouble(vm, 0, total);
}

WrenForeignMethodFn foreignConstructorBindMethod(const char* signature)
{
  if (strcmp(signature, "Adder.total") == 0) return adderTotal;

  return NULL;
}

void foreignConstructorBindClass(
    const char* className, WrenForeignClassMethods* methods)
{
  if (strcmp(className, "Adder") == 0)
  {
    methods->allocate = adderAllocate;
    return;
  }
}
