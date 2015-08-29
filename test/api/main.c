#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "wren.h"

#include "foreign_class.h"
#include "returns.h"
#include "value.h"

#define REGISTER_METHOD(name, camelCase) \
  if (strcmp(testName, #name) == 0) return camelCase##BindMethod(fullName)

#define REGISTER_CLASS(name, camelCase) \
    if (strcmp(testName, #name) == 0) \
    { \
      camelCase##BindClass(className, &methods); \
    }

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

  REGISTER_METHOD(foreign_class, foreignClass);
  REGISTER_METHOD(returns, returns);
  REGISTER_METHOD(value, value);

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

  REGISTER_CLASS(foreign_class, foreignClass);

  return methods;
}


int main(int argc, const char* argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: wren <test>\n");
    return 64; // EX_USAGE.
  }

  testName = argv[1];

  // The test script is at "test/api/<test>.wren".
  char testPath[256];
  strcpy(testPath, "test/api/");
  strcat(testPath, testName);
  strcat(testPath, ".wren");

  setForeignCallbacks(bindForeignMethod, bindForeignClass);
  runFile(testPath);
  return 0;
}
