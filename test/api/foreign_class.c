#include <stdio.h>
#include <string.h>

#include "foreign_class.h"

static int finalized = 0;

static void apiFinalized(WrenFiber* fiber)
{
  wrenSetSlotDouble(fiber, 0, finalized);
}

static void counterAllocate(WrenFiber* fiber)
{
  double* value = (double*)wrenSetSlotNewForeign(fiber, 0, 0, sizeof(double));
  *value = 0;
}

static void counterIncrement(WrenFiber* fiber)
{
  double* value = (double*)wrenGetSlotForeign(fiber, 0);
  double increment = wrenGetSlotDouble(fiber, 1);

  *value += increment;
}

static void counterValue(WrenFiber* fiber)
{
  double value = *(double*)wrenGetSlotForeign(fiber, 0);
  wrenSetSlotDouble(fiber, 0, value);
}

static void pointAllocate(WrenFiber* fiber)
{
  double* coordinates = (double*)wrenSetSlotNewForeign(fiber, 0, 0, sizeof(double[3]));

  // This gets called by both constructors, so sniff the slot count to see
  // which one was invoked.
  if (wrenGetSlotCount(fiber) == 1)
  {
    coordinates[0] = 0.0;
    coordinates[1] = 0.0;
    coordinates[2] = 0.0;
  }
  else
  {
    coordinates[0] = wrenGetSlotDouble(fiber, 1);
    coordinates[1] = wrenGetSlotDouble(fiber, 2);
    coordinates[2] = wrenGetSlotDouble(fiber, 3);
  }
}

static void pointTranslate(WrenFiber* fiber)
{
  double* coordinates = (double*)wrenGetSlotForeign(fiber, 0);
  coordinates[0] += wrenGetSlotDouble(fiber, 1);
  coordinates[1] += wrenGetSlotDouble(fiber, 2);
  coordinates[2] += wrenGetSlotDouble(fiber, 3);
}

static void pointToString(WrenFiber* fiber)
{
  double* coordinates = (double*)wrenGetSlotForeign(fiber, 0);
  char result[100];
  sprintf(result, "(%g, %g, %g)",
      coordinates[0], coordinates[1], coordinates[2]);
  wrenSetSlotString(fiber, 0, result);
}

static void resourceAllocate(WrenFiber* fiber)
{
  int* value = (int*)wrenSetSlotNewForeign(fiber, 0, 0, sizeof(int));
  *value = 123;
}

static void resourceFinalize(void* data)
{
  // Make sure we get the right data back.
  int* value = (int*)data;
  if (*value != 123) exit(1);
  
  finalized++;
}

WrenForeignMethodFn foreignClassBindMethod(const char* signature)
{
  if (strcmp(signature, "static ForeignClass.finalized") == 0) return apiFinalized;
  if (strcmp(signature, "Counter.increment(_)") == 0) return counterIncrement;
  if (strcmp(signature, "Counter.value") == 0) return counterValue;
  if (strcmp(signature, "Point.translate(_,_,_)") == 0) return pointTranslate;
  if (strcmp(signature, "Point.toString") == 0) return pointToString;

  return NULL;
}

void foreignClassBindClass(
    const char* className, WrenForeignClassMethods* methods)
{
  if (strcmp(className, "Counter") == 0)
  {
    methods->allocate = counterAllocate;
    return;
  }

  if (strcmp(className, "Point") == 0)
  {
    methods->allocate = pointAllocate;
    return;
  }

  if (strcmp(className, "Resource") == 0)
  {
    methods->allocate = resourceAllocate;
    methods->finalize = resourceFinalize;
    return;
  }
}
