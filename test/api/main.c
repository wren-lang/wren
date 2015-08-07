#include <stdio.h>
#include <string.h>

#include "io.h"
#include "vm.h"
#include "wren.h"

#include "get_value/get_value.h"
#include "return_bool/return_bool.h"
#include "return_double/return_double.h"
#include "return_null/return_null.h"

#define REGISTER_TEST(name, camelCase) \
  if (strcmp(testName, #name) == 0) return camelCase##BindForeign(fullName)

// The name of the currently executing API test.
const char* testName;

static WrenForeignMethodFn bindForeign(
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

  REGISTER_TEST(get_value, getValue);
  REGISTER_TEST(return_bool, returnBool);
  REGISTER_TEST(return_double, returnDouble);
  REGISTER_TEST(return_null, returnNull);

  fprintf(stderr,
      "Unknown foreign method '%s' for test '%s'\n", fullName, testName);
  exit(1);
  return NULL;
}

int main(int argc, const char* argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: wren <test>\n");
    return 64; // EX_USAGE.
  }

  testName = argv[1];

  // The test script is at "test/api/<test>/<test>.wren".
  char testPath[256];
  strcpy(testPath, "test/api/");
  strcat(testPath, testName);
  strcat(testPath, "/");
  strcat(testPath, testName);
  strcat(testPath, ".wren");

  runFile(bindForeign, testPath);
  return 0;
}
