#include <string.h>

#include "call.h"
#include "vm.h"

void callRunTests(WrenVM* vm)
{
  wrenEnsureSlots(vm, 1);
  wrenGetVariable(vm, "main", "Call", 0);
  WrenValue* callClass = wrenGetSlotValue(vm, 0);
  
  WrenValue* noParams = wrenMakeCallHandle(vm, "noParams");
  WrenValue* zero = wrenMakeCallHandle(vm, "zero()");
  WrenValue* one = wrenMakeCallHandle(vm, "one(_)");
  WrenValue* two = wrenMakeCallHandle(vm, "two(_,_)");
  WrenValue* unary = wrenMakeCallHandle(vm, "-");
  WrenValue* binary = wrenMakeCallHandle(vm, "-(_)");
  WrenValue* subscript = wrenMakeCallHandle(vm, "[_,_]");
  WrenValue* subscriptSet = wrenMakeCallHandle(vm, "[_,_]=(_)");

  // Different arity.
  wrenEnsureSlots(vm, 1);
  wrenSetSlotValue(vm, 0, callClass);
  wrenCall(vm, noParams);
  
  wrenEnsureSlots(vm, 1);
  wrenSetSlotValue(vm, 0, callClass);
  wrenCall(vm, zero);
  
  wrenEnsureSlots(vm, 2);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenCall(vm, one);
  
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, two);
  
  // Operators.
  wrenEnsureSlots(vm, 1);
  wrenSetSlotValue(vm, 0, callClass);
  wrenCall(vm, unary);

  wrenEnsureSlots(vm, 2);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenCall(vm, binary);
  
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenCall(vm, subscript);
  
  wrenEnsureSlots(vm, 4);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.0);
  wrenSetSlotDouble(vm, 2, 2.0);
  wrenSetSlotDouble(vm, 3, 3.0);
  wrenCall(vm, subscriptSet);

  // Returning a value.
  WrenValue* getValue = wrenMakeCallHandle(vm, "getValue()");
  wrenEnsureSlots(vm, 1);
  wrenSetSlotValue(vm, 0, callClass);
  wrenCall(vm, getValue);
  WrenValue* value = wrenGetSlotValue(vm, 0);
  
  // Different argument types.
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotBool(vm, 1, true);
  wrenSetSlotBool(vm, 2, false);
  wrenCall(vm, two);

  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotDouble(vm, 1, 1.2);
  wrenSetSlotDouble(vm, 2, 3.4);
  wrenCall(vm, two);
  
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotString(vm, 1, "string");
  wrenSetSlotString(vm, 2, "another");
  wrenCall(vm, two);
  
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotNull(vm, 1);
  wrenSetSlotValue(vm, 2, value);
  wrenCall(vm, two);
  
  // Truncate a string, or allow null bytes.
  wrenEnsureSlots(vm, 3);
  wrenSetSlotValue(vm, 0, callClass);
  wrenSetSlotBytes(vm, 1, "string", 3);
  wrenSetSlotBytes(vm, 2, "b\0y\0t\0e", 7);
  wrenCall(vm, two);
  
  // Call ignores with extra temporary slots on stack.
  wrenEnsureSlots(vm, 10);
  wrenSetSlotValue(vm, 0, callClass);
  for (int i = 1; i < 10; i++)
  {
    wrenSetSlotDouble(vm, i, i * 0.1);
  }
  wrenCall(vm, one);
  
  wrenReleaseValue(vm, callClass);
  wrenReleaseValue(vm, noParams);
  wrenReleaseValue(vm, zero);
  wrenReleaseValue(vm, one);
  wrenReleaseValue(vm, two);
  wrenReleaseValue(vm, getValue);
  wrenReleaseValue(vm, value);
  wrenReleaseValue(vm, unary);
  wrenReleaseValue(vm, binary);
  wrenReleaseValue(vm, subscript);
  wrenReleaseValue(vm, subscriptSet);
}
