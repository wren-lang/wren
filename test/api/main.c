#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "wren.h"

#include "benchmark.h"
#include "call.h"
#include "foreign_class.h"
#include "lists.h"
#include "slots.h"
#include "value.h"

// The name of the currently executing API test.
const char* testName;

static WrenForeignMethodFn bindForeignMethod(
    WrenVM* vm, const char* module, const char* className,
    bool isStatic, const char* signature)
{  
  if (strcmp(module, "main") != 0) return NULL;

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
  
  method = foreignClassBindMethod(fullName);
  if (method != NULL) return method;
  
  method = listsBindMethod(fullName);
  if (method != NULL) return method;
  
  method = slotsBindMethod(fullName);
  if (method != NULL) return method;
  
  method = valueBindMethod(fullName);
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
  if (strcmp(module, "main") != 0) return methods;

  foreignClassBindClass(className, &methods);

  return methods;
}

static void afterLoad(WrenVM* vm) {
  if (strstr(testName, "/call.wren") != NULL) callRunTests(vm);
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
  return 0;
}
