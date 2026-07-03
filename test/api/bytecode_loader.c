#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bytecode_loader.h"

typedef struct
{
  WrenConfiguration config;
  int errorsReported;
  WrenVM* vm;
} TestContext;

static void reportError(WrenVM* vm, WrenErrorType type, const char* module,
                        int line, const char* message)
{
  (void)vm;
  (void)module;
  (void)line;
  (void)message;
  (void)type;
}

static WrenConfiguration testConfig(void)
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.errorFn = reportError;
  return config;
}

static TestContext newContext(void)
{
  TestContext ctx;
  ctx.config = testConfig();
  ctx.errorsReported = 0;
  ctx.vm = wrenNewVM(&ctx.config);
  return ctx;
}

static void freeContext(TestContext* ctx)
{
  wrenFreeVM(ctx->vm);
}

static bool expectResult(WrenInterpretResult actual,
                         WrenInterpretResult expected,
                         const char* name)
{
  if (actual == expected) return true;

  fprintf(stderr, "%s: expected result %d, got %d\n", name, expected, actual);
  return false;
}

static bool expect(bool condition, const char* message)
{
  if (condition) return true;

  fprintf(stderr, "%s\n", message);
  return false;
}

static bool roundTrip(void)
{
  TestContext ctx = newContext();

  const char* source =
      "var x = 1\n"
      "var y = 2\n"
      "System.print(x + y)\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!expect(serialized.bytes != NULL, "roundTrip: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "main",
      serialized.bytes, serialized.length);
  bool ok = expectResult(result, WREN_RESULT_SUCCESS, "roundTrip");

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool alreadyLoaded(void)
{
  TestContext ctx = newContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!expect(serialized.bytes != NULL, "alreadyLoaded: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  WrenInterpretResult first = wrenInterpretBytecode(ctx.vm, "loaded",
      serialized.bytes, serialized.length);
  bool ok = expectResult(first, WREN_RESULT_SUCCESS, "alreadyLoaded first");

  WrenInterpretResult second = wrenInterpretBytecode(ctx.vm, "loaded",
      serialized.bytes, serialized.length);
  ok = expectResult(second, WREN_RESULT_LOAD_ERROR, "alreadyLoaded second") && ok;

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool failedLoadIsRetryable(void)
{
  TestContext ctx = newContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!expect(serialized.bytes != NULL,
              "failedLoadIsRetryable: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  // Truncate the artifact so the load fails partway through.
  size_t truncatedLength = serialized.length / 2;

  WrenInterpretResult fail = wrenInterpretBytecode(ctx.vm, "retry",
      serialized.bytes, truncatedLength);
  bool ok = expectResult(fail, WREN_RESULT_LOAD_ERROR,
                         "failedLoadIsRetryable fail");

  if (ok)
  {
    // The same module name should still be available for a successful load.
    WrenInterpretResult success = wrenInterpretBytecode(ctx.vm, "retry",
        serialized.bytes, serialized.length);
    ok = expectResult(success, WREN_RESULT_SUCCESS,
                      "failedLoadIsRetryable success");
  }

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool badHeaderRejects(void)
{
  TestContext ctx = newContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!expect(serialized.bytes != NULL, "badHeaderRejects: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  uint8_t badMagic[8];
  memcpy(badMagic, serialized.bytes, 8);
  badMagic[0] = 'X';

  uint8_t badVersion[8];
  memcpy(badVersion, serialized.bytes, 8);
  badVersion[4] = 99;

  uint8_t badFlags[8];
  memcpy(badFlags, serialized.bytes, 8);
  badFlags[7] = 0xFF;

  bool ok = expectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badMagic, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects magic");
  ok = ok && expectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badVersion, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects version");
  ok = ok && expectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badFlags, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects flags");

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool trailingBytesReject(void)
{
  TestContext ctx = newContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!expect(serialized.bytes != NULL,
              "trailingBytesReject: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  uint8_t* trailing = (uint8_t*)malloc(serialized.length + 1);
  memcpy(trailing, serialized.bytes, serialized.length);
  trailing[serialized.length] = 0;

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "trailing",
      trailing, serialized.length + 1);
  bool ok = expectResult(result, WREN_RESULT_LOAD_ERROR, "trailingBytesReject");

  free(trailing);
  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool runtimeErrorIsReported(void)
{
  TestContext ctx = newContext();

  const char* source =
      "System.print(\"before\")\n"
      "var n = 1 + true\n"
      "System.print(\"after\")\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, false);
  if (!expect(serialized.bytes != NULL,
              "runtimeErrorIsReported: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "runtime",
      serialized.bytes, serialized.length);
  bool ok = expectResult(result, WREN_RESULT_RUNTIME_ERROR,
                         "runtimeErrorIsReported");

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static bool deepNesting(void)
{
  TestContext ctx = newContext();

  const char* source =
      "var make\n"
      "make = Fn.new { |n|\n"
      "  if (n == 0) return Fn.new { System.print(\"done\") }\n"
      "  var outer = n\n"
      "  var child = make.call(n - 1)\n"
      "  return Fn.new {\n"
      "    System.print(outer)\n"
      "    child.call()\n"
      "  }\n"
      "}\n"
      "make.call(10).call()\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, false);
  if (!expect(serialized.bytes != NULL, "deepNesting: serialization failed"))
  {
    freeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "nested",
      serialized.bytes, serialized.length);
  bool ok = expectResult(result, WREN_RESULT_SUCCESS, "deepNesting");

  wrenFreeSerializeResult(&ctx.config, serialized);
  freeContext(&ctx);
  return ok;
}

static void runTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;
  ok = roundTrip() && ok;
  ok = alreadyLoaded() && ok;
  ok = failedLoadIsRetryable() && ok;
  ok = badHeaderRejects() && ok;
  ok = trailingBytesReject() && ok;
  ok = runtimeErrorIsReported() && ok;
  ok = deepNesting() && ok;

  wrenSetSlotBool(vm, 0, ok);
}

WrenForeignMethodFn bytecodeLoaderBindMethod(const char* signature)
{
  if (strcmp(signature, "static BytecodeLoader.runTests()") == 0) return runTests;

  return NULL;
}
