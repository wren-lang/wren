#pragma once
#include "wren.h"
#include <stddef.h>
#include <stdbool.h>

typedef struct {
  char* data;
  size_t length;
  size_t capacity;
} OutputBuffer;

typedef struct {
  WrenConfiguration config;
  int errorsReported;
  char lastError[256];
  WrenVM* vm;
  OutputBuffer output;
} TestContext;

WrenConfiguration btTestConfig(void);
void btNewContext(TestContext* ctx);
void btFreeContext(TestContext* ctx);
void btResetContext(TestContext* ctx);
int btErrorsReported(TestContext* ctx);
const char* btLastError(TestContext* ctx);
const char* btOutput(TestContext* ctx);
bool btExpectResult(WrenInterpretResult actual, WrenInterpretResult expected, const char* name);
bool btExpect(bool condition, const char* message);
bool btExpectStringEq(const char* actual, const char* expected, const char* name);
bool btRunEquivalence(const char* source, bool debugInfo,
                        WrenInterpretResult expectedResult,
                        const char* expectedOutput);
