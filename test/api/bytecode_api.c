#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// API/host-behavior tests
// ---------------------------------------------------------------------------

static bool moduleIsRegistered(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "moduleIsRegistered: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "loaded",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "moduleIsRegistered load");
  ok = btExpect(wrenHasModule(ctx.vm, "loaded"), "moduleIsRegistered: loaded module missing") && ok;

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool variableAccess(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source =
      "var exported = 42\n"
      "System.print(exported)\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "variableAccess: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "vars",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "variableAccess load");

  ok = btExpect(wrenHasModule(ctx.vm, "vars"), "variableAccess: module missing") && ok;
  ok = btExpect(wrenHasVariable(ctx.vm, "vars", "exported"),
              "variableAccess: exported variable missing") && ok;

  wrenEnsureSlots(ctx.vm, 1);
  wrenGetVariable(ctx.vm, "vars", "exported", 0);
  ok = btExpect(wrenGetSlotDouble(ctx.vm, 0) == 42.0,
              "variableAccess: variable value mismatch") && ok;

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool callExportedMethod(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source =
      "var greet = Fn.new { System.print(\"hi\") }\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "callExportedMethod: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "callmod",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "callExportedMethod load");

  // Look up the exported variable and invoke it as a Fn via call().
  wrenEnsureSlots(ctx.vm, 1);
  wrenGetVariable(ctx.vm, "callmod", "greet", 0);
  WrenHandle* callHandle = wrenMakeCallHandle(ctx.vm, "call()");
  result = wrenCall(ctx.vm, callHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS, "callExportedMethod call") && ok;
  ok = btExpectStringEq(btOutput(&ctx), "hi\n", "callExportedMethod output") && ok;

  wrenReleaseHandle(ctx.vm, callHandle);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static void recordLoadErrorOnly(WrenVM* vm, WrenErrorType type, const char* module,
                                int line, const char* message)
{
  (void)vm;
  (void)module;
  (void)line;
  (void)message;

  if (type == WREN_ERROR_LOAD)
  {
    int* count = (int*)wrenGetUserData(vm);
    if (count != NULL) (*count)++;
  }
}

static bool errorCallback(void)
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  int loadErrors = 0;
  config.userData = &loadErrors;
  config.errorFn = recordLoadErrorOnly;
  config.writeFn = NULL;

  WrenVM* vm = wrenNewVM(&config);

  uint8_t bad[8] = { 'W', 'R', 'E', 'N', 0, 0, 0, 0xFF };
  WrenInterpretResult result = wrenInterpretBytecode(vm, "bad", bad, sizeof(bad));
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR, "errorCallback result");
  ok = btExpect(loadErrors > 0, "errorCallback: load error was not reported") && ok;

  wrenFreeVM(vm);
  return ok;
}

static bool serializeFailureIsCompileError(void)
{
  // Invalid source should fail serialization, returning NULL bytes.
  WrenConfiguration config;
  wrenInitConfiguration(&config);

  const char* source = "var 1 = 2\n";
  WrenSerializeResult serialized = wrenSerializeModule(&config, "main", source, true);
  bool ok = btExpect(serialized.bytes == NULL,
                   "serializeFailureIsCompileError: expected NULL bytes");
  wrenFreeSerializeResult(&config, serialized);
  return ok;
}

static bool alreadyLoaded(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "alreadyLoaded: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult first = wrenInterpretBytecode(ctx.vm, "loaded",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(first, WREN_RESULT_SUCCESS, "alreadyLoaded first");

  WrenInterpretResult second = wrenInterpretBytecode(ctx.vm, "loaded",
      serialized.bytes, serialized.length);
  ok = btExpectResult(second, WREN_RESULT_LOAD_ERROR, "alreadyLoaded second") && ok;

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool failedLoadIsRetryable(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
              "failedLoadIsRetryable: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  // Truncate the artifact so the load fails partway through.
  size_t truncatedLength = serialized.length / 2;

  WrenInterpretResult fail = wrenInterpretBytecode(ctx.vm, "retry",
      serialized.bytes, truncatedLength);
  bool ok = btExpectResult(fail, WREN_RESULT_LOAD_ERROR,
                           "failedLoadIsRetryable fail");

  if (ok)
  {
    // The same module name should still be available for a successful load.
    WrenInterpretResult success = wrenInterpretBytecode(ctx.vm, "retry",
        serialized.bytes, serialized.length);
    ok = btExpectResult(success, WREN_RESULT_SUCCESS,
                        "failedLoadIsRetryable success");
  }

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool writeFnIsUsed(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "System.print(\"output\")\n";

  WrenConfiguration serializerConfig;
  wrenInitConfiguration(&serializerConfig);

  WrenSerializeResult serialized = wrenSerializeModule(&serializerConfig, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "writeFnIsUsed: serialization failed"))
  {
    wrenFreeSerializeResult(&serializerConfig, serialized);
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "main",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "writeFnIsUsed load");
  ok = btExpectStringEq(btOutput(&ctx), "output\n", "writeFnIsUsed output") && ok;

  wrenFreeSerializeResult(&serializerConfig, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool callLoadedClassMethods(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source =
      "class Greeter {\n"
      "  construct new() {}\n"
      "  static greet(name) { System.print(\"static \" + name) }\n"
      "  hello(name) { System.print(\"instance \" + name) }\n"
      "}\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
               "callLoadedClassMethods: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "classmod",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "callLoadedClassMethods load");

  wrenEnsureSlots(ctx.vm, 2);
  wrenGetVariable(ctx.vm, "classmod", "Greeter", 0);

  // Static method call: greet("hi").
  WrenHandle* greetHandle = wrenMakeCallHandle(ctx.vm, "greet(_)");
  wrenSetSlotString(ctx.vm, 1, "hi");
  result = wrenCall(ctx.vm, greetHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                      "callLoadedClassMethods static") && ok;
  ok = btExpectStringEq(btOutput(&ctx), "static hi\n",
                       "callLoadedClassMethods static output") && ok;

  // Constructor call: new(). The receiver must be the class, which was replaced
  // by the static call's return value, so reload it.
  wrenEnsureSlots(ctx.vm, 2);
  wrenGetVariable(ctx.vm, "classmod", "Greeter", 0);
  WrenHandle* newHandle = wrenMakeCallHandle(ctx.vm, "new()");
  result = wrenCall(ctx.vm, newHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                       "callLoadedClassMethods construct") && ok;

  // Instance method call: hello("there") on the instance left in slot 0.
  wrenEnsureSlots(ctx.vm, 2);
  wrenSetSlotString(ctx.vm, 1, "there");
  WrenHandle* helloHandle = wrenMakeCallHandle(ctx.vm, "hello(_)");
  result = wrenCall(ctx.vm, helloHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                       "callLoadedClassMethods instance") && ok;
  ok = btExpectStringEq(btOutput(&ctx), "static hi\ninstance there\n",
                       "callLoadedClassMethods instance output") && ok;

  wrenReleaseHandle(ctx.vm, greetHandle);
  wrenReleaseHandle(ctx.vm, newHandle);
  wrenReleaseHandle(ctx.vm, helloHandle);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool preexistingMethodSymbols(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  // Bump the loading VM's method-symbol table so that interned indices no
  // longer match the serializer VM's indices.
  WrenHandle* shiftHandle = wrenMakeCallHandle(ctx.vm, "preexisting(_)");
  wrenReleaseHandle(ctx.vm, shiftHandle);
  btResetContext(&ctx);

  // Re-create the shift handle after reset to ensure the fresh VM also has an
  // entry ahead of the loaded method names.
  shiftHandle = wrenMakeCallHandle(ctx.vm, "preexisting(_)");

  const char* source =
      "class Greeter {\n"
      "  construct new() {}\n"
      "  static greet(name) { System.print(\"static \" + name) }\n"
      "  hello(name) { System.print(\"instance \" + name) }\n"
      "}\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
               "preexistingMethodSymbols: serialization failed"))
  {
    wrenReleaseHandle(ctx.vm, shiftHandle);
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "prevm",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                            "preexistingMethodSymbols load");

  wrenEnsureSlots(ctx.vm, 2);
  wrenGetVariable(ctx.vm, "prevm", "Greeter", 0);

  WrenHandle* greetHandle = wrenMakeCallHandle(ctx.vm, "greet(_)");
  wrenSetSlotString(ctx.vm, 1, " shifted");
  result = wrenCall(ctx.vm, greetHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                       "preexistingMethodSymbols static") && ok;
  ok = btExpectStringEq(btOutput(&ctx), "static  shifted\n",
                       "preexistingMethodSymbols static output") && ok;

  // The static call replaced slot 0 with its return value, so reload the class.
  wrenEnsureSlots(ctx.vm, 2);
  wrenGetVariable(ctx.vm, "prevm", "Greeter", 0);
  wrenSetSlotString(ctx.vm, 1, "works");
  WrenHandle* newHandle = wrenMakeCallHandle(ctx.vm, "new()");
  result = wrenCall(ctx.vm, newHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                       "preexistingMethodSymbols construct") && ok;

  wrenEnsureSlots(ctx.vm, 2);
  wrenSetSlotString(ctx.vm, 1, "still");
  WrenHandle* helloHandle = wrenMakeCallHandle(ctx.vm, "hello(_)");
  result = wrenCall(ctx.vm, helloHandle);
  ok = btExpectResult(result, WREN_RESULT_SUCCESS,
                       "preexistingMethodSymbols instance") && ok;
  ok = btExpectStringEq(btOutput(&ctx),
                       "static  shifted\ninstance still\n",
                       "preexistingMethodSymbols instance output") && ok;

  wrenReleaseHandle(ctx.vm, shiftHandle);
  wrenReleaseHandle(ctx.vm, greetHandle);
  wrenReleaseHandle(ctx.vm, newHandle);
  wrenReleaseHandle(ctx.vm, helloHandle);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

bool bytecodeAPIRunTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;

  ok = moduleIsRegistered() && ok;
  ok = variableAccess() && ok;
  ok = callExportedMethod() && ok;
  ok = callLoadedClassMethods() && ok;
  ok = preexistingMethodSymbols() && ok;
  ok = errorCallback() && ok;
  ok = serializeFailureIsCompileError() && ok;
  ok = alreadyLoaded() && ok;
  ok = failedLoadIsRetryable() && ok;
  ok = writeFnIsUsed() && ok;

  return ok;
}
