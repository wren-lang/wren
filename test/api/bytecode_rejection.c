#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ---------------------------------------------------------------------------
// Malformed-artifact mutation helpers
// ---------------------------------------------------------------------------

// The serialized artifact uses big-endian multibyte integers.
typedef struct
{
  size_t arityOffset;
  size_t numUpvaluesOffset;
  size_t maxSlotsOffset;
  size_t codeLengthOffset;
  size_t constantCountOffset;
  size_t firstConstantTagOffset;
  size_t debugNameLengthOffset;
  size_t lineCountOffset;
} MinimalLayout;

static uint32_t readUint32BE(const uint8_t* bytes)
{
  return ((uint32_t)bytes[0] << 24) |
         ((uint32_t)bytes[1] << 16) |
         ((uint32_t)bytes[2] << 8) |
         (uint32_t)bytes[3];
}

static uint16_t readUint16BE(const uint8_t* bytes)
{
  return (uint16_t)(((uint16_t)bytes[0] << 8) | (uint16_t)bytes[1]);
}

static bool skipString(const uint8_t* bytes, size_t length, size_t* off)
{
  if (*off + 4 > length) return false;
  uint32_t len = readUint32BE(bytes + *off);
  *off += 4;
  if (*off + len > length) return false;
  *off += len;
  return true;
}

static bool skipConstant(const uint8_t* bytes, size_t length, size_t* off);

static bool skipFunctionBody(const uint8_t* bytes, size_t length, size_t* off)
{
  if (*off + 4 > length) return false;
  uint32_t codeLength = readUint32BE(bytes + *off);
  *off += 4;
  if (*off + codeLength > length) return false;
  *off += codeLength;

  if (*off + 4 > length) return false;
  uint32_t constantCount = readUint32BE(bytes + *off);
  *off += 4;

  for (uint32_t i = 0; i < constantCount; i++)
  {
    if (!skipConstant(bytes, length, off)) return false;
  }

  // Debug name.
  if (!skipString(bytes, length, off)) return false;

  // Source lines.
  if (*off + 4 > length) return false;
  uint32_t lineCount = readUint32BE(bytes + *off);
  *off += 4;
  if (*off + (size_t)lineCount * 4 > length) return false;
  *off += (size_t)lineCount * 4;

  return true;
}

static bool skipConstant(const uint8_t* bytes, size_t length, size_t* off)
{
  if (*off + 1 > length) return false;
  uint8_t tag = bytes[*off];
  *off += 1;

  switch (tag)
  {
    case 0: // CONSTANT_NULL
    case 1: // CONSTANT_FALSE
    case 2: // CONSTANT_TRUE
      return true;

    case 3: // CONSTANT_NUM
      if (*off + 8 > length) return false;
      *off += 8;
      return true;

    case 4: // CONSTANT_STRING
      return skipString(bytes, length, off);

    case 5: // CONSTANT_FN
    {
      if (*off + 1 + 2 + 4 > length) return false;
      *off += 1 + 2 + 4; // arity, numUpvalues, maxSlots.
      return skipFunctionBody(bytes, length, off);
    }

    default:
      return false;
  }
}

static bool parseMinimalLayout(const uint8_t* bytes, size_t length,
                               MinimalLayout* out)
{
  memset(out, 0, sizeof(*out));

  if (length < 8) return false;
  bool debugInfo = (bytes[7] & 0x01) != 0;
  size_t off = 8;

  if (off + 4 > length) return false;
  uint32_t ownVarCount = readUint32BE(bytes + off);
  off += 4;

  for (uint32_t i = 0; i < ownVarCount; i++)
  {
    if (!skipString(bytes, length, &off)) return false;
  }

  out->arityOffset = off;
  if (off + 1 > length) return false;
  off += 1;

  out->numUpvaluesOffset = off;
  if (off + 2 > length) return false;
  off += 2;

  out->maxSlotsOffset = off;
  if (off + 4 > length) return false;
  off += 4;

  out->codeLengthOffset = off;
  if (off + 4 > length) return false;
  uint32_t codeLength = readUint32BE(bytes + off);
  off += 4;

  if (off + codeLength > length) return false;
  off += codeLength;

  out->constantCountOffset = off;
  if (off + 4 > length) return false;
  uint32_t constantCount = readUint32BE(bytes + off);
  off += 4;

  if (constantCount > 0)
  {
    out->firstConstantTagOffset = off;
    if (off + 1 > length) return false;
  }

  // Skip constants to reach debug info for debug artifacts.
  if (debugInfo)
  {
    for (uint32_t i = 0; i < constantCount; i++)
    {
      if (!skipConstant(bytes, length, &off)) return false;
    }

    out->debugNameLengthOffset = off;
    if (!skipString(bytes, length, &off)) return false;

    out->lineCountOffset = off;
  }

  return true;
}

// Serializes a minimal artifact and parses its layout. On failure the test
// context is freed and NULL is returned.
static WrenSerializeResult serializeMinimalArtifact(TestContext* ctx,
                                                    MinimalLayout* layout)
{
  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx->config, "main",
                                                       source, true);
  if (!btExpect(serialized.bytes != NULL,
              "serializeMinimalArtifact: serialization failed"))
  {
    return serialized;
  }

  if (!btExpect(parseMinimalLayout(serialized.bytes, serialized.length, layout),
              "serializeMinimalArtifact: could not parse artifact layout"))
  {
    wrenFreeSerializeResult(&ctx->config, serialized);
    serialized.bytes = NULL;
    serialized.length = 0;
    return serialized;
  }

  return serialized;
}

// Loads a mutated copy of [serialized] and asserts it is rejected.
static bool expectLoadErrorForMutation(TestContext* ctx,
                                       const WrenSerializeResult* serialized,
                                       const char* testName)
{
  WrenInterpretResult result = wrenInterpretBytecode(ctx->vm, testName,
      serialized->bytes, serialized->length);
  if (result != WREN_RESULT_LOAD_ERROR)
  {
    fprintf(stderr, "%s: expected WREN_RESULT_LOAD_ERROR, got %d\n",
            testName, result);
    return false;
  }
  return true;
}

// Builds a new artifact buffer from [serialized] with the first own variable
// name replaced by [name] and [nameLength]. Returns a freshly allocated buffer
// that the caller must free, or NULL on failure.
static uint8_t* buildArtifactWithVariableName(const WrenSerializeResult* serialized,
                                              const char* newName,
                                              size_t newNameLength,
                                              size_t* outLength)
{
  if (serialized->length < 16) return NULL;

  uint32_t originalNameLength = readUint32BE(serialized->bytes + 12);
  size_t headerBeforeNameLength = 12 + 4; // ownVariableCount + name length field.
  size_t headerAfterName = headerBeforeNameLength + originalNameLength;

  size_t tailLength = serialized->length - headerAfterName;
  size_t newLength = headerBeforeNameLength + newNameLength + tailLength;

  uint8_t* result = (uint8_t*)malloc(newLength);
  if (result == NULL) return NULL;

  memcpy(result, serialized->bytes, headerBeforeNameLength);
  result[12] = (uint8_t)(newNameLength >> 24);
  result[13] = (uint8_t)(newNameLength >> 16);
  result[14] = (uint8_t)(newNameLength >> 8);
  result[15] = (uint8_t)(newNameLength);
  memcpy(result + headerBeforeNameLength, newName, newNameLength);
  memcpy(result + headerBeforeNameLength + newNameLength,
         serialized->bytes + headerAfterName, tailLength);

  *outLength = newLength;
  return result;
}

// ---------------------------------------------------------------------------
// Loader-rejection / malformed-artifact tests
// ---------------------------------------------------------------------------

static bool truncationSweep(void)
{
  TestContext ctx = btNewContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "truncationSweep: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  bool ok = true;
  for (size_t len = 0; len < serialized.length; len++)
  {
    WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "trunc",
        serialized.bytes, len);
    if (result != WREN_RESULT_LOAD_ERROR)
    {
      fprintf(stderr,
              "truncationSweep: expected load error at length %zu, got %d\n",
              len, result);
      ok = false;
      break;
    }
  }

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool unknownHeaderFlags(void)
{
  TestContext ctx = btNewContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "unknownHeaderFlags: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  uint8_t badFlags[8];
  memcpy(badFlags, serialized.bytes, 8);
  badFlags[7] = 0xFF;

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "flags", badFlags, 8);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR, "unknownHeaderFlags");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

// ---------------------------------------------------------------------------
// Malformed-artifact mutation tests
// ---------------------------------------------------------------------------

static bool invalidFunctionArity(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  serialized.bytes[layout.arityOffset] = 99;
  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "invalidFunctionArity");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool invalidMaxSlots(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // maxSlots is stored big-endian; set it to 0.
  serialized.bytes[layout.maxSlotsOffset + 0] = 0;
  serialized.bytes[layout.maxSlotsOffset + 1] = 0;
  serialized.bytes[layout.maxSlotsOffset + 2] = 0;
  serialized.bytes[layout.maxSlotsOffset + 3] = 0;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "invalidMaxSlots");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool rootNumUpvaluesNonZero(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // numUpvalues is big-endian; set it to 1.
  serialized.bytes[layout.numUpvaluesOffset + 0] = 0;
  serialized.bytes[layout.numUpvaluesOffset + 1] = 1;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "rootNumUpvaluesNonZero");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool impossibleCodeLength(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  serialized.bytes[layout.codeLengthOffset + 0] = 0xFF;
  serialized.bytes[layout.codeLengthOffset + 1] = 0xFF;
  serialized.bytes[layout.codeLengthOffset + 2] = 0xFF;
  serialized.bytes[layout.codeLengthOffset + 3] = 0xFF;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "impossibleCodeLength");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool invalidConstantTag(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  if (!btExpect(layout.firstConstantTagOffset != 0,
              "invalidConstantTag: no constant tag to mutate"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  serialized.bytes[layout.firstConstantTagOffset] = 0xFF;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "invalidConstantTag");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool mismatchedDebugLineCount(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  if (!btExpect(layout.lineCountOffset != 0,
              "mismatchedDebugLineCount: no line count to mutate"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  serialized.bytes[layout.lineCountOffset + 0] = 0;
  serialized.bytes[layout.lineCountOffset + 1] = 0;
  serialized.bytes[layout.lineCountOffset + 2] = 0;
  serialized.bytes[layout.lineCountOffset + 3] = 99;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "mismatchedDebugLineCount");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool emptyVariableName(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // The first own variable name length field sits at bytes 12-15.
  serialized.bytes[12] = 0;
  serialized.bytes[13] = 0;
  serialized.bytes[14] = 0;
  serialized.bytes[15] = 0;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                       "emptyVariableName");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool duplicateVariableName(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // Insert a second own variable named "x" right after the first one.
  size_t originalNameLength = readUint32BE(serialized.bytes + 12);
  size_t firstNameEnd = 16 + originalNameLength;
  size_t tailLength = serialized.length - firstNameEnd;
  size_t newLength = firstNameEnd + 4 + 1 + tailLength;

  uint8_t* mutant = (uint8_t*)malloc(newLength);
  if (!btExpect(mutant != NULL, "duplicateVariableName: out of memory"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  memcpy(mutant, serialized.bytes, firstNameEnd);
  mutant[firstNameEnd + 0] = 0;
  mutant[firstNameEnd + 1] = 0;
  mutant[firstNameEnd + 2] = 0;
  mutant[firstNameEnd + 3] = 1;
  mutant[firstNameEnd + 4] = 'x';
  memcpy(mutant + firstNameEnd + 5, serialized.bytes + firstNameEnd, tailLength);

  // Update own variable count to 2.
  mutant[8] = 0;
  mutant[9] = 0;
  mutant[10] = 0;
  mutant[11] = 2;

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm,
      "dupvar", mutant, newLength);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR,
                           "duplicateVariableName");

  free(mutant);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool coreNameCollision(void)
{
  TestContext ctx = btNewContext();
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  size_t mutantLength;
  uint8_t* mutant = buildArtifactWithVariableName(&serialized, "System", 6,
                                                  &mutantLength);
  if (!btExpect(mutant != NULL, "coreNameCollision: failed to build mutant"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm,
      "corecol", mutant, mutantLength);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR,
                           "coreNameCollision");

  free(mutant);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

// ---------------------------------------------------------------------------
// Additional rejection / debug-error tests
// ---------------------------------------------------------------------------

static bool badHeaderRejects(void)
{
  TestContext ctx = btNewContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "badHeaderRejects: serialization failed"))
  {
    btFreeContext(&ctx);
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

  bool ok = btExpectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badMagic, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects magic");
  ok = ok && btExpectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badVersion, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects version");
  ok = ok && btExpectResult(
      wrenInterpretBytecode(ctx.vm, "bad", badFlags, 8),
      WREN_RESULT_LOAD_ERROR, "badHeaderRejects flags");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool trailingBytesReject(void)
{
  TestContext ctx = btNewContext();

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
              "trailingBytesReject: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  uint8_t* trailing = (uint8_t*)malloc(serialized.length + 1);
  memcpy(trailing, serialized.bytes, serialized.length);
  trailing[serialized.length] = 0;

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "trailing",
      trailing, serialized.length + 1);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR, "trailingBytesReject");

  free(trailing);
  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool runtimeErrorIsReported(void)
{
  TestContext ctx = btNewContext();

  const char* source =
      "System.print(\"before\")\n"
      "var n = 1 + true\n"
      "System.print(\"after\")\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, false);
  if (!btExpect(serialized.bytes != NULL,
              "runtimeErrorIsReported: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "runtime",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_RUNTIME_ERROR,
                           "runtimeErrorIsReported");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

bool bytecodeRejectionRunTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;

  ok = badHeaderRejects() && ok;
  ok = trailingBytesReject() && ok;
  ok = truncationSweep() && ok;
  ok = unknownHeaderFlags() && ok;
  ok = invalidFunctionArity() && ok;
  ok = invalidMaxSlots() && ok;
  ok = rootNumUpvaluesNonZero() && ok;
  ok = impossibleCodeLength() && ok;
  ok = invalidConstantTag() && ok;
  ok = mismatchedDebugLineCount() && ok;
  ok = emptyVariableName() && ok;
  ok = duplicateVariableName() && ok;
  ok = coreNameCollision() && ok;
  ok = runtimeErrorIsReported() && ok;

  return ok;
}
