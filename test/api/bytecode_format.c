#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Format/serializer sanity tests
// ---------------------------------------------------------------------------

static bool debugFlagMatchesRequest(void)
{
  WrenConfiguration config;
  wrenInitConfiguration(&config);

  const char* source = "var x = 1\n";

  WrenSerializeResult debug = wrenSerializeModule(&config, "main", source, true);
  WrenSerializeResult stripped = wrenSerializeModule(&config, "main", source, false);

  bool ok = btExpect(debug.bytes != NULL, "debugFlagMatchesRequest: debug serialize failed");
  ok = btExpect(stripped.bytes != NULL, "debugFlagMatchesRequest: stripped serialize failed") && ok;

  if (ok)
  {
    // Header: magic (4) + version (3) + flags (1). The debug-info flag is bit 0.
    ok = btExpect((debug.bytes[7] & 0x01) != 0,
                "debugFlagMatchesRequest: debug flag not set") && ok;
    ok = btExpect((stripped.bytes[7] & 0x01) == 0,
                "debugFlagMatchesRequest: stripped flag unexpectedly set") && ok;
  }

  wrenFreeSerializeResult(&config, debug);
  wrenFreeSerializeResult(&config, stripped);
  return ok;
}

static bool moduleNameOverride(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source = "var x = 1\n";
  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "original", source, true);
  if (!btExpect(serialized.bytes != NULL, "moduleNameOverride: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "renamed",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "moduleNameOverride load");
  ok = btExpect(wrenHasModule(ctx.vm, "renamed"),
              "moduleNameOverride: renamed module not registered") && ok;
  ok = btExpect(!wrenHasModule(ctx.vm, "original"),
              "moduleNameOverride: original module should not be registered") && ok;

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool roundTrip(void)
{
  TestContext ctx;
  btNewContext(&ctx);

  const char* source =
      "var x = 1\n"
      "var y = 2\n"
      "System.print(x + y)\n";

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "roundTrip: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "main",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "roundTrip");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

static bool freshContextLoadsOutput(void)
{
  WrenConfiguration serializerConfig;
  wrenInitConfiguration(&serializerConfig);

  const char* source =
      "var msg = \"hello\"\n"
      "System.print(msg)\n";

  WrenSerializeResult serialized = wrenSerializeModule(&serializerConfig, "main", source, true);
  if (!btExpect(serialized.bytes != NULL, "freshContextLoadsOutput: serialization failed"))
  {
    wrenFreeSerializeResult(&serializerConfig, serialized);
    return false;
  }

  TestContext ctx;
  btNewContext(&ctx);

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "main",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "freshContextLoadsOutput load");
  ok = btExpectStringEq(btOutput(&ctx), "hello\n", "freshContextLoadsOutput output") && ok;

  wrenFreeSerializeResult(&serializerConfig, serialized);
  btFreeContext(&ctx);
  return ok;
}

bool bytecodeFormatRunTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;

  ok = debugFlagMatchesRequest() && ok;
  ok = moduleNameOverride() && ok;
  ok = roundTrip() && ok;
  ok = freshContextLoadsOutput() && ok;

  return ok;
}
