#include <string.h>

#include "call.h"
#include "vm.h"

static void runTests(WrenVM* vm)
{
  WrenValue* noParams = wrenGetMethod(vm, "main", "Api", "noParams");
  WrenValue* zero = wrenGetMethod(vm, "main", "Api", "zero()");
  WrenValue* one = wrenGetMethod(vm, "main", "Api", "one(_)");
  WrenValue* two = wrenGetMethod(vm, "main", "Api", "two(_,_)");
  
  // Different arity.
  wrenCall(vm, noParams, NULL, "");
  wrenCall(vm, zero, NULL, "");
  wrenCall(vm, one, NULL, "i", 1);
  wrenCall(vm, two, NULL, "ii", 1, 2);

  WrenValue* getValue = wrenGetMethod(vm, "main", "Api", "getValue(_)");
  
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


WrenForeignMethodFn callBindMethod(const char* signature)
{
  if (strcmp(signature, "static Api.runTests()") == 0) return runTests;

  return NULL;
}

