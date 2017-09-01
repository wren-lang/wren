#include <string.h>

#include "user_data.h"

static const char* data = "my user data";
static const char* otherData = "other user data";

static void test(WrenVM* vm)
{
  WrenConfiguration configuration;
  wrenInitConfiguration(&configuration);
  
  // Should default to NULL.
  if (configuration.userData != NULL)
  {
    wrenSetSlotBool(vm, 0, false);
    return;
  }
  
  configuration.userData = (void*)data;
  
  WrenVM* otherVM = wrenNewVM(&configuration);
  
  // Should be able to get it.
  if (wrenGetUserData(otherVM) != data)
  {
    wrenSetSlotBool(vm, 0, false);
    wrenFreeVM(otherVM);
    return;
  }
  
  // Should be able to set it.
  wrenSetUserData(otherVM, (void*)otherData);
  
  if (wrenGetUserData(otherVM) != otherData)
  {
    wrenSetSlotBool(vm, 0, false);
    wrenFreeVM(otherVM);
    return;
  }

  wrenSetSlotBool(vm, 0, true);
  wrenFreeVM(otherVM);
}

WrenForeignMethodFn userDataBindMethod(const char* signature)
{
  if (strcmp(signature, "static UserData.test") == 0) return test;

  return NULL;
}
