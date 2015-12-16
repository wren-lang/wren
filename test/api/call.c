#include <string.h>

#include "call.h"
#include "vm.h"

void callRunTests(WrenVM* vm)
{
  WrenValue* noParams = wrenGetMethod(vm, "main", "Call", "noParams");
  WrenValue* zero = wrenGetMethod(vm, "main", "Call", "zero()");
  WrenValue* one = wrenGetMethod(vm, "main", "Call", "one(_)");
  WrenValue* two = wrenGetMethod(vm, "main", "Call", "two(_,_)");
  
  // Different arity.
  wrenCall(vm, noParams, NULL, "");
  wrenCall(vm, zero, NULL, "");
  wrenCall(vm, one, NULL, "i", 1);
  wrenCall(vm, two, NULL, "ii", 1, 2);

  WrenValue* getValue = wrenGetMethod(vm, "main", "Call", "getValue(_)");
  
  // Returning a value.
  WrenValue* value = NULL;
  wrenCall(vm, getValue, &value, "v", NULL);
  
  // Different argument types.
  wrenCall(vm, two, NULL, "bb", true, false);
  wrenCall(vm, two, NULL, "dd", 1.2, 3.4);
  wrenCall(vm, two, NULL, "ii", 3, 4);
  wrenCall(vm, two, NULL, "ss", "string", "another");
  wrenCall(vm, two, NULL, "vv", NULL, value);
  
  // Truncate a string, or allow null bytes.
  wrenCall(vm, two, NULL, "aa", "string", 3, "b\0y\0t\0e", 7);
  
  wrenReleaseValue(vm, noParams);
  wrenReleaseValue(vm, zero);
  wrenReleaseValue(vm, one);
  wrenReleaseValue(vm, two);
  wrenReleaseValue(vm, getValue);
  wrenReleaseValue(vm, value);
}
