#include <string.h>

#include "call.h"
#include "vm.h"

void callRunTests(WrenVM* vm)
{
  wrenSetSlotCount(vm, 1);
  wrenGetVariable(vm, "./test/api/call", "Call", 0);
  WrenHandle* callClass = wrenGetSlotHandle(vm, 0);
  
  WrenHandle* noParams = wrenMakeCallHandle(vm, "noParams");
  WrenHandle* zero = wrenMakeCallHandle(vm, "zero()");
  WrenHandle* one = wrenMakeCallHandle(vm, "one(_)");
  WrenHandle* two = wrenMakeCallHandle(vm, "two(_,_)");
  WrenHandle* unary = wrenMakeCallHandle(vm, "-");
  WrenHandle* binary = wrenMakeCallHandle(vm, "-(_)");
  WrenHandle* subscript = wrenMakeCallHandle(vm, "[_,_]");
  WrenHandle* subscriptSet = wrenMakeCallHandle(vm, "[_,_]=(_)");

  // Different arity.
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenCall(vm, noParams);
  
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenCall(vm, zero);
  
  wrenSetSlotCount(vm, 2);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenCall(vm, one);
  
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, two);
  
  // Operators.
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenCall(vm, unary);

  wrenSetSlotCount(vm, 2);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenCall(vm, binary);
  
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, subscript);
  
  wrenSetSlotCount(vm, 4);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenSetSlotDouble(vm, 3, 3.0);
  wrenCall(vm, subscriptSet);

  // Returning a value.
  WrenHandle* getValue = wrenMakeCallHandle(vm, "getValue()");
  wrenSetSlotCount(vm, 1);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenCall(vm, getValue);
  WrenHandle* value = wrenGetSlotHandle(vm, 0);
  
  // Different argument types.
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotBool(vm, 1, true);
  wrenSetSlotBool(vm, 2, false);
  wrenCall(vm, two);

  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.2);
  wrenSetSlotDouble(vm, 2, 3.4);
  wrenCall(vm, two);
  
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotString(vm, 1, "string");
  wrenSetSlotString(vm, 2, "another");
  wrenCall(vm, two);
  
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotNull(vm, 1);
  wrenSetSlotHandle(vm, 2, value);
  wrenCall(vm, two);
  
  // Truncate a string, or allow null bytes.
  wrenSetSlotCount(vm, 3);
  wrenSetSlotHandle(vm, 0, callClass);
  wrenSetSlotBytes(vm, 1, "string", 3);
  wrenSetSlotBytes(vm, 2, "b\0y\0t\0e", 7);
  wrenCall(vm, two);
  
  // Call ignores with extra temporary slots on stack.
  wrenSetSlotCount(vm, 10);
  wrenSetSlotHandle(vm, 0, callClass);
  for (int i = 1; i < 10; i++)
  {
    wrenSetSlotDouble(vm, i, i * 0.1);
  }
  wrenCall(vm, one);
  
  wrenReleaseHandle(vm, callClass);
  wrenReleaseHandle(vm, noParams);
  wrenReleaseHandle(vm, zero);
  wrenReleaseHandle(vm, one);
  wrenReleaseHandle(vm, two);
  wrenReleaseHandle(vm, getValue);
  wrenReleaseHandle(vm, value);
  wrenReleaseHandle(vm, unary);
  wrenReleaseHandle(vm, binary);
  wrenReleaseHandle(vm, subscript);
  wrenReleaseHandle(vm, subscriptSet);
}
