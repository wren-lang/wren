#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void appendOutput(WrenVM* vm, const char* text)
{
  TestContext* ctx = (TestContext*)wrenGetUserData(vm);
  if (ctx == NULL) return;

  size_t len = strlen(text);
  size_t needed = ctx->output.length + len + 1;
  if (needed > ctx->output.capacity)
  {
    ctx->output.capacity = needed * 2;
    if (ctx->output.capacity == 0) ctx->output.capacity = 256;
    ctx->output.data = (char*)realloc(ctx->output.data, ctx->output.capacity);
  }

  memcpy(ctx->output.data + ctx->output.length, text, len);
  ctx->output.length += len;
  ctx->output.data[ctx->output.length] = '\0';
}

static void reportError(WrenVM* vm, WrenErrorType type, const char* module,
                         int line, const char* message)
{
  TestContext* ctx = (TestContext*)wrenGetUserData(vm);
  if (ctx != NULL)
  {
    ctx->errorsReported++;
    // Keep the most recent error message for tests that want to inspect it.
    // Stack-trace callbacks do not carry the actual error text, so ignore them.
    if (message != NULL && type != WREN_ERROR_STACK_TRACE)
    {
      strncpy(ctx->lastError, message, sizeof(ctx->lastError) - 1);
      ctx->lastError[sizeof(ctx->lastError) - 1] = '\0';
    }
  }

  (void)type;
  (void)module;
  (void)line;
}


WrenConfiguration btTestConfig(void)
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);
  config.errorFn = reportError;
  config.writeFn = appendOutput;
  return config;
}

void btNewContext(TestContext* ctx)
{
  ctx->config = btTestConfig();
  ctx->errorsReported = 0;
  ctx->lastError[0] = '\0';
  ctx->output.data = NULL;
  ctx->output.length = 0;
  ctx->output.capacity = 0;
  ctx->config.userData = ctx;
  ctx->vm = wrenNewVM(&ctx->config);
}

void btFreeContext(TestContext* ctx)
{
  wrenFreeVM(ctx->vm);
  free(ctx->output.data);
}

void btResetContext(TestContext* ctx)
{
  // Free and recreate the VM so a previous run cannot pollute the next one,
  // but keep the same config/userData.
  wrenFreeVM(ctx->vm);
  ctx->output.length = 0;
  if (ctx->output.data != NULL) ctx->output.data[0] = '\0';
  ctx->errorsReported = 0;
  ctx->lastError[0] = '\0';
  ctx->vm = wrenNewVM(&ctx->config);
}

int btErrorsReported(TestContext* ctx)
{
  return ctx->errorsReported;
}

const char* btLastError(TestContext* ctx)
{
  return ctx->lastError;
}

const char* btOutput(TestContext* ctx)
{
  return ctx->output.data != NULL ? ctx->output.data : "";
}

bool btExpectResult(WrenInterpretResult actual,
                    WrenInterpretResult expected,
                    const char* name)
{
  if (actual == expected) return true;

  fprintf(stderr, "%s: expected result %d, got %d\n", name, expected, actual);
  return false;
}

bool btExpect(bool condition, const char* message)
{
  if (condition) return true;

  fprintf(stderr, "%s\n", message);
  return false;
}

bool btExpectStringEq(const char* actual, const char* expected,
                      const char* what)
{
  if (expected == NULL) return true;
  if (actual != NULL && strcmp(actual, expected) == 0) return true;

  fprintf(stderr, "%s: expected '%s', got '%s'\n",
          what, expected != NULL ? expected : "(null)",
          actual != NULL ? actual : "(null)");
  return false;
}

// Serializes [source], then runs both the source directly and the serialized
// artifact and checks that both produce [expectedResult] and the same output.
// It also checks that both outputs equal [expectedOutput] when non-NULL.
bool btRunEquivalence(const char* source, bool debugInfo,
                      WrenInterpretResult expectedResult,
                      const char* expectedOutput)
{
  static const char* moduleName = "equiv";

  WrenConfiguration serializeConfig;
  wrenInitConfiguration(&serializeConfig);
  WrenSerializeResult serialized = wrenSerializeModule(&serializeConfig, moduleName,
                                                     source, debugInfo);
  if (!btExpect(serialized.bytes != NULL, "btRunEquivalence: serialization failed"))
  {
    wrenFreeSerializeResult(&serializeConfig, serialized);
    return false;
  }

  // Run from source.
  TestContext sourceCtx;
  btNewContext(&sourceCtx);
  WrenInterpretResult sourceResult = wrenInterpret(sourceCtx.vm, moduleName, source);
  const char* sourceOutput = btOutput(&sourceCtx);

  bool ok = btExpectResult(sourceResult, expectedResult, "btRunEquivalence source result");
  ok = btExpectStringEq(sourceOutput, expectedOutput, "btRunEquivalence source output") && ok;

  // Run from bytecode.
  TestContext byteCtx;
  btNewContext(&byteCtx);
  WrenInterpretResult byteResult = wrenInterpretBytecode(byteCtx.vm, moduleName,
      serialized.bytes, serialized.length);
  const char* byteOutput = btOutput(&byteCtx);

  ok = btExpectResult(byteResult, expectedResult, "btRunEquivalence bytecode result") && ok;

  if (strcmp(sourceOutput, byteOutput) != 0)
  {
    fprintf(stderr, "btRunEquivalence: source vs bytecode output mismatch\n"
                    "  source:   %s\n  bytecode: %s\n", sourceOutput, byteOutput);
    ok = false;
  }

  ok = btExpectStringEq(byteOutput, expectedOutput, "btRunEquivalence bytecode output") && ok;

  wrenFreeSerializeResult(&serializeConfig, serialized);
  btFreeContext(&sourceCtx);
  btFreeContext(&byteCtx);
  return ok;
}
