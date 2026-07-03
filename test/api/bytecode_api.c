#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// API/host-behavior tests
// ---------------------------------------------------------------------------

static bool moduleIsRegistered(void)
{
  TestContext ctx = btNewContext();

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
  TestContext ctx = btNewContext();

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
  TestContext ctx = btNewContext();

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
  TestContext ctx = btNewContext();

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
  TestContext ctx = btNewContext();

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
  TestContext ctx = btNewContext();

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

bool bytecodeAPIRunTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;

  ok = moduleIsRegistered() && ok;
  ok = variableAccess() && ok;
  ok = callExportedMethod() && ok;
  ok = errorCallback() && ok;
  ok = serializeFailureIsCompileError() && ok;
  ok = alreadyLoaded() && ok;
  ok = failedLoadIsRetryable() && ok;
  ok = writeFnIsUsed() && ok;

  return ok;
}
