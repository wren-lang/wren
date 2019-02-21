#include "wren.h"
#include "example.h"
#include <stdio.h>

void exampleAdd(WrenVM *vm)
{
  double a = wrenGetSlotDouble(vm, 1);
  double b = wrenGetSlotDouble(vm, 2);
  wrenSetSlotDouble(vm, 0, a + b); 
  printf("called exampleAdd %G\n", a+b);
}


void exampleStr(WrenVM *vm)
{
  const char* a = wrenGetSlotString(vm, 1);
  const char* b = wrenGetSlotString(vm, 2);
  printf("called exampleStr %s %s\n", a,b);
}
