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
  size_t methodNameCountOffset;
  size_t firstMethodNameLengthOffset;
  size_t ownVarCountOffset;
  size_t firstOwnVarNameLengthOffset;
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

  // Skip the method-name symbol table.
  out->methodNameCountOffset = off;
  if (off + 4 > length) return false;
  uint32_t methodNameCount = readUint32BE(bytes + off);
  off += 4;

  if (methodNameCount > 0)
  {
    out->firstMethodNameLengthOffset = off;
  }

  for (uint32_t i = 0; i < methodNameCount; i++)
  {
    if (!skipString(bytes, length, &off)) return false;
  }

  // Skip the module's own variable names.
  out->ownVarCountOffset = off;
  if (off + 4 > length) return false;
  uint32_t ownVarCount = readUint32BE(bytes + off);
  off += 4;

  out->firstOwnVarNameLengthOffset = off;
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

// Builds a new artifact buffer from [serialized] with the variable name at
// [nameOffset] replaced by [name] and [nameLength]. Returns a freshly
// allocated buffer that the caller must free, or NULL on failure.
static uint8_t* buildArtifactWithVariableName(const WrenSerializeResult* serialized,
                                              size_t nameOffset,
                                              const char* newName,
                                              size_t newNameLength,
                                              size_t* outLength)
{
  if (serialized->length < nameOffset + 4) return NULL;

  uint32_t originalNameLength = readUint32BE(serialized->bytes + nameOffset);
  size_t headerBeforeNameLength = nameOffset + 4;
  size_t headerAfterName = headerBeforeNameLength + originalNameLength;

  if (headerAfterName > serialized->length) return NULL;

  size_t tailLength = serialized->length - headerAfterName;
  size_t newLength = headerBeforeNameLength + newNameLength + tailLength;

  uint8_t* result = (uint8_t*)malloc(newLength);
  if (result == NULL) return NULL;

  memcpy(result, serialized->bytes, headerBeforeNameLength);
  result[nameOffset + 0] = (uint8_t)(newNameLength >> 24);
  result[nameOffset + 1] = (uint8_t)(newNameLength >> 16);
  result[nameOffset + 2] = (uint8_t)(newNameLength >> 8);
  result[nameOffset + 3] = (uint8_t)(newNameLength);
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
  TestContext ctx;
  btNewContext(&ctx);

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
  TestContext ctx;
  btNewContext(&ctx);

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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
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
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // The first own variable name length field sits after the method table.
  serialized.bytes[layout.firstOwnVarNameLengthOffset + 0] = 0;
  serialized.bytes[layout.firstOwnVarNameLengthOffset + 1] = 0;
  serialized.bytes[layout.firstOwnVarNameLengthOffset + 2] = 0;
  serialized.bytes[layout.firstOwnVarNameLengthOffset + 3] = 0;

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                        "emptyVariableName");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}


static bool duplicateVariableName(void)
{
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  // Insert a second own variable named "x" right after the first one.
  size_t originalNameLength = readUint32BE(
      serialized.bytes + layout.firstOwnVarNameLengthOffset);
  size_t firstNameEnd = layout.firstOwnVarNameLengthOffset + 4 + originalNameLength;
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

  // Update own variable count.
  size_t ownCountOff = layout.ownVarCountOffset;
  mutant[ownCountOff + 0] = 0;
  mutant[ownCountOff + 1] = 0;
  mutant[ownCountOff + 2] = 0;
  mutant[ownCountOff + 3] = 2;

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
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  size_t mutantLength;
  uint8_t* mutant = buildArtifactWithVariableName(&serialized,
      layout.firstOwnVarNameLengthOffset, "System", 6, &mutantLength);
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
// Method-symbol table rejection tests
// ---------------------------------------------------------------------------

static void writeUint32BE(uint8_t* bytes, uint32_t value)
{
  bytes[0] = (uint8_t)(value >> 24);
  bytes[1] = (uint8_t)(value >> 16);
  bytes[2] = (uint8_t)(value >> 8);
  bytes[3] = (uint8_t)(value);
}

static void writeUint16BE(uint8_t* bytes, uint16_t value)
{
  bytes[0] = (uint8_t)(value >> 8);
  bytes[1] = (uint8_t)(value);
}

static bool truncatedMethodNameCount(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
               "truncatedMethodNameCount: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "tmc",
      serialized.bytes, 9);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR,
                            "truncatedMethodNameCount");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool excessiveMethodNameCount(void)
{
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  writeUint32BE(serialized.bytes + layout.methodNameCountOffset, 0xFFFFFFFF);

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                        "excessiveMethodNameCount");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool emptyMethodName(void)
{
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  writeUint32BE(serialized.bytes + layout.methodNameCountOffset, 1);
  writeUint32BE(serialized.bytes + layout.firstMethodNameLengthOffset, 0);

  bool ok = expectLoadErrorForMutation(&ctx, &serialized, "emptyMethodName");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool truncatedMethodNameString(void)
{
  TestContext ctx;
  btNewContext(&ctx);
  MinimalLayout layout;
  WrenSerializeResult serialized = serializeMinimalArtifact(&ctx, &layout);
  if (serialized.bytes == NULL)
  {
    btFreeContext(&ctx);
    return false;
  }

  writeUint32BE(serialized.bytes + layout.methodNameCountOffset, 1);
  writeUint32BE(serialized.bytes + layout.firstMethodNameLengthOffset, 10);
  // The artifact now claims a 10-byte name but provides none after the
  // length field, so the loader should reject it as truncated.
  serialized.bytes[layout.firstMethodNameLengthOffset + 4] = 'a';

  bool ok = expectLoadErrorForMutation(&ctx, &serialized,
                                        "truncatedMethodNameString");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool outOfRangeMethodOperand(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source =
      "class Greeter {\n"
      "  static greet(name) { System.print(\"static \" + name) }\n"
      "}\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL,
               "outOfRangeMethodOperand: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  MinimalLayout layout;
  if (!btExpect(parseMinimalLayout(serialized.bytes, serialized.length, &layout),
               "outOfRangeMethodOperand: could not parse layout"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  uint32_t codeLength = readUint32BE(serialized.bytes + layout.codeLengthOffset);
  size_t codeStart = layout.codeLengthOffset + 4;
  if (!btExpect(codeStart + codeLength <= serialized.length,
               "outOfRangeMethodOperand: code section out of bounds"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  // Find the first CODE_METHOD_STATIC (72) byte in the root function's code.
  size_t opcodeOffset = (size_t)-1;
  for (size_t i = 0; i < codeLength; i++)
  {
    if (serialized.bytes[codeStart + i] == 72)
    {
      opcodeOffset = codeStart + i;
      break;
    }
  }

  if (!btExpect(opcodeOffset != (size_t)-1,
               "outOfRangeMethodOperand: no method static opcode found"))
  {
    wrenFreeSerializeResult(&ctx.config, serialized);
    btFreeContext(&ctx);
    return false;
  }

  // Patch the two-byte method-symbol operand to a value larger than the
  // serialized method-name table could possibly contain.
  writeUint16BE(serialized.bytes + opcodeOffset + 1, 0xFFFF);

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "oor",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_LOAD_ERROR,
                            "outOfRangeMethodOperand");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

// ---------------------------------------------------------------------------
// Additional rejection / debug-error tests
// ---------------------------------------------------------------------------

static bool badHeaderRejects(void)
{
  TestContext ctx;
  btNewContext(&ctx);

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
  TestContext ctx;
  btNewContext(&ctx);

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
  TestContext ctx;
  btNewContext(&ctx);

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
  ok = truncatedMethodNameCount() && ok;
  ok = excessiveMethodNameCount() && ok;
  ok = emptyMethodName() && ok;
  ok = truncatedMethodNameString() && ok;
  ok = outOfRangeMethodOperand() && ok;
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
