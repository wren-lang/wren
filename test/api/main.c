#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "wren.h"

#include "benchmark.h"
#include "call.h"
#include "call_wren_call_root.h"
#include "error.h"
#include "get_variable.h"
#include "foreign_class.h"
#include "handle.h"
#include "lists.h"
#include "new_vm.h"
#include "reset_stack_after_call_abort.h"
#include "reset_stack_after_foreign_construct.h"
#include "resolution.h"
#include "slots.h"
#include "user_data.h"

// The name of the currently executing API test.
const char* testName;

static WrenForeignMethodFn bindForeignMethod(
    WrenVM* vm, const char* module, const char* className,
    bool isStatic, const char* signature)
{
  if (strncmp(module, "./test/", 7) != 0) return NULL;

  // For convenience, concatenate all of the method qualifiers into a single
  // signature string.
  char fullName[256];
  fullName[0] = '\0';
  if (isStatic) strcat(fullName, "static ");
  strcat(fullName, className);
  strcat(fullName, ".");
  strcat(fullName, signature);
  
  WrenForeignMethodFn method = NULL;
  
  method = benchmarkBindMethod(fullName);
  if (method != NULL) return method;
  
  method = errorBindMethod(fullName);
  if (method != NULL) return method;

  method = getVariableBindMethod(fullName);
  if (method != NULL) return method;

  method = foreignClassBindMethod(fullName);
  if (method != NULL) return method;
  
  method = handleBindMethod(fullName);
  if (method != NULL) return method;

  method = listsBindMethod(fullName);
  if (method != NULL) return method;
  
  method = newVMBindMethod(fullName);
  if (method != NULL) return method;
  
  method = resolutionBindMethod(fullName);
  if (method != NULL) return method;

  method = slotsBindMethod(fullName);
  if (method != NULL) return method;
  
  method = userDataBindMethod(fullName);
  if (method != NULL) return method;

  fprintf(stderr,
      "Unknown foreign method '%s' for test '%s'\n", fullName, testName);
  exit(1);
  return NULL;
}

static WrenForeignClassMethods bindForeignClass(
    WrenVM* vm, const char* module, const char* className)
{
  WrenForeignClassMethods methods = { NULL, NULL };
  if (strncmp(module, "./test/", 7) != 0) return methods;

  foreignClassBindClass(className, &methods);
  if (methods.allocate != NULL) return methods;
  
  resetStackAfterForeignConstructBindClass(className, &methods);
  if (methods.allocate != NULL) return methods;
  
  slotsBindClass(className, &methods);
  if (methods.allocate != NULL) return methods;
  
  fprintf(stderr,
          "Unknown foreign class '%s' for test '%s'\n", className, testName);
  exit(1);
  return methods;
}

static void afterLoad(WrenVM* vm)
{
  if (strstr(testName, "/call.wren") != NULL)
  {
    callRunTests(vm);
  }
  else if (strstr(testName, "/call_wren_call_root.wren") != NULL)
  {
    callWrenCallRootRunTests(vm);
  }
  else if (strstr(testName, "/reset_stack_after_call_abort.wren") != NULL)
  {
    resetStackAfterCallAbortRunTests(vm);
  }
  else if (strstr(testName, "/reset_stack_after_foreign_construct.wren") != NULL)
  {
    resetStackAfterForeignConstructRunTests(vm);
  }
}

int main(int argc, const char* argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: wren <test>\n");
    return 64; // EX_USAGE.
  }

  testName = argv[1];
  setTestCallbacks(bindForeignMethod, bindForeignClass, afterLoad);
  runFile(testName);
  return getExitCode();
}
