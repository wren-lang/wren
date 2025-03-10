#include <stdio.h>
#include <string.h>

#include "raw_value.h"

static int holderCounter = 0;

typedef struct {
  WrenRawValue val;
} RawValueHolder;


static void holderAllocate(WrenVM* vm)
{
  RawValueHolder* holder = wrenSetSlotNewForeign(vm, 0, 0, sizeof(RawValueHolder));
  holder->val = wrenNullRawValue();
  holderCounter++;
}

static size_t holderTrace(WrenTracer* trace, void* p)
{
  RawValueHolder* holder = p;
  wrenTraceRawValue(trace, holder->val);

  return sizeof(*holder);
}


static void holderFinalizer(void* p)
{
  holderCounter--;
}

static void holderSet(WrenVM* vm)
{
  RawValueHolder* holder = wrenGetSlotForeign(vm, 0);
  holder->val = wrenGetSlotRawValue(vm, 1);
}

static void holderGet(WrenVM* vm)
{
  RawValueHolder* holder = wrenGetSlotForeign(vm, 0);
  wrenSetSlotRawValue(vm, 0, holder->val);
}

static void holderCount(WrenVM* vm)
{
  wrenSetSlotDouble(vm, 0, (double)holderCounter);
}

WrenForeignMethodFn rawValueBindMethod(const char* signature)
{
  if (strcmp(signature, "RawValueHolder.value") == 0) return holderGet;
  if (strcmp(signature, "RawValueHolder.value=(_)") == 0) return holderSet;
  if (strcmp(signature, "static RawValueHolder.count") == 0) return holderCount;

  return NULL;
}

void rawValueBindClass(
    const char* className, WrenForeignClassMethods* methods)
{
  if (strcmp(className, "RawValueHolder") == 0)
  {
    methods->allocate = holderAllocate;
    methods->trace    = holderTrace;
    methods->finalize = holderFinalizer;
  }
}
