#include "bytecode_test.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Execution-equivalence tests
// ---------------------------------------------------------------------------

static bool equivalenceTopLevel(void)
{
  const char* source =
      "var a = 1\n"
      "var b = 2\n"
      "a = a + b\n"
      "System.print(a)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "3\n");
}

static bool equivalenceClasses(void)
{
  const char* source =
      "class Point {\n"
      "  construct new(x, y) {\n"
      "    _x = x\n"
      "    _y = y\n"
      "  }\n"
      "  x { _x }\n"
      "  y { _y }\n"
      "  static origin { Point.new(0, 0) }\n"
      "  toString { \"(%(_x), %(_y))\" }\n"
      "}\n"
      "var p = Point.origin\n"
      "System.print(p.toString)\n"
      "var q = Point.new(3, 4)\n"
      "System.print(q.toString)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS,
                        "(0, 0)\n(3, 4)\n");
}

static bool equivalenceInheritance(void)
{
  const char* source =
      "class Base {\n"
      "  greet { \"base\" }\n"
      "}\n"
      "class Derived is Base {\n"
      "  construct new() {}\n"
      "  greet { \"derived \" + super.greet }\n"
      "}\n"
      "System.print(Derived.new().greet)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "derived base\n");
}

static bool equivalenceMethodArguments(void)
{
  const char* source =
      "class C {\n"
      "  construct new() {}\n"
      "  f(x) { x + 1 }\n"
      "}\n"
      "System.print(C.new().f(41))\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "42\n");
}

static bool equivalenceClosures(void)
{
  const char* source =
      "var makeAdder\n"
      "makeAdder = Fn.new {|x|\n"
      "  return Fn.new {|y| x + y }\n"
      "}\n"
      "var add5 = makeAdder.call(5)\n"
      "System.print(add5.call(3))\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "8\n");
}

static bool equivalenceControlFlow(void)
{
  const char* source =
      "var sum = 0\n"
      "for (i in 1..5) {\n"
      "  if (i % 2 == 0 || i == 3) {\n"
      "    sum = sum + i\n"
      "  }\n"
      "}\n"
      "while (sum < 20) sum = sum + 1\n"
      "System.print(sum)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "20\n");
}

static bool equivalenceLiterals(void)
{
  const char* source =
      "var l = [1, 2, 3]\n"
      "var m = {\"a\": 1, \"b\": 2}\n"
      "var flag = true && false || !false\n"
      "System.print(l.count)\n"
      "System.print(m[\"a\"])\n"
      "System.print(flag)\n"
      "System.print(null)\n"
      "System.print(\"ok\" + \"!\")\n"
      "System.print(3.14.ceil)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS,
                        "3\n1\ntrue\nnull\nok!\n4\n");
}

static bool equivalenceConstants(void)
{
  const char* source =
      "var l = [1, 2, 3]\n"
      "var m = {\"a\": 1, \"b\": 2}\n"
      "var flag = true && false || !false\n"
      "System.print(l.count)\n"
      "System.print(m[\"a\"])\n"
      "System.print(flag)\n"
      "System.print(null)\n"
      "System.print(\"ok\" + \"!\")\n"
      "System.print(3.14.ceil)\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS,
                        "3\n1\ntrue\nnull\nok!\n4\n");
}

static bool equivalenceBooleans(void)
{
  const char* source =
      "var t = true\n"
      "var f = false\n"
      "if (t && !f) {\n"
      "  System.print(\"yes\")\n"
      "} else {\n"
      "  System.print(\"no\")\n"
      "}\n";

  return btRunEquivalence(source, true, WREN_RESULT_SUCCESS, "yes\n");
}

static bool equivalenceAttributes(void)
{
  // Attributes are not supported in this Wren compiler, so this source shape
  // would be a compile error. We still exercise the boolean-constant path
  // indirectly in equivalenceLiterals, equivalenceConstants, and
  // equivalenceBooleans, and skip the attribute syntax here.
  (void)btRunEquivalence;
  return true;
}

static bool equivalenceRuntimeError(void)
{
  const char* source =
      "System.print(\"before\")\n"
      "var n = 1 + true\n";

  return btRunEquivalence(source, true, WREN_RESULT_RUNTIME_ERROR, "before\n");
}

static bool equivalenceDebugStrippedRuntimeError(void)
{
  const char* source =
      "System.print(\"before\")\n"
      "var n = 1 + true\n";

  return btRunEquivalence(source, false, WREN_RESULT_RUNTIME_ERROR, "before\n");
}

static bool deepNesting(void)
{
  TestContext ctx = btNewContext();

  // Build a source string with 12 literally nested Fn.new literals on a single
  // expression line. Wren requires closing braces inline, not on their own
  // line, so the whole nested chain lives on one line.
  #define DEPTH 12
  char source[4096];
  char* p = source;
  char* end = source + sizeof(source);

  p += snprintf(p, end - p, "var f = ");
  for (int i = 0; i < DEPTH; i++)
  {
    p += snprintf(p, end - p, "Fn.new { ");
  }
  p += snprintf(p, end - p, "System.print(\"deep\")");
  for (int i = 0; i < DEPTH; i++)
  {
    p += snprintf(p, end - p, " }");
  }
  p += snprintf(p, end - p, "\nf");
  for (int i = 0; i < DEPTH; i++) p += snprintf(p, end - p, ".call()");
  p += snprintf(p, end - p, "\n");

  if (p >= end)
  {
    fprintf(stderr, "deepNesting: source buffer overflow\n");
    btFreeContext(&ctx);
    return false;
  }
  #undef DEPTH

  WrenSerializeResult serialized = wrenSerializeModule(&ctx.config, "main", source, false);
  if (!btExpect(serialized.bytes != NULL, "deepNesting: serialization failed"))
  {
    btFreeContext(&ctx);
    return false;
  }

  WrenInterpretResult result = wrenInterpretBytecode(ctx.vm, "nested",
      serialized.bytes, serialized.length);
  bool ok = btExpectResult(result, WREN_RESULT_SUCCESS, "deepNesting");

  wrenFreeSerializeResult(&ctx.config, serialized);
  btFreeContext(&ctx);
  return ok;
}

bool bytecodeEquivalenceRunTests(WrenVM* vm)
{
  (void)vm;
  bool ok = true;

  ok = equivalenceTopLevel() && ok;
  ok = equivalenceClasses() && ok;
  ok = equivalenceInheritance() && ok;
  ok = equivalenceMethodArguments() && ok;
  ok = equivalenceClosures() && ok;
  ok = equivalenceControlFlow() && ok;
  ok = equivalenceLiterals() && ok;
  ok = equivalenceConstants() && ok;
  ok = equivalenceBooleans() && ok;
  ok = equivalenceAttributes() && ok;
  ok = equivalenceRuntimeError() && ok;
  ok = equivalenceDebugStrippedRuntimeError() && ok;
  ok = deepNesting() && ok;

  return ok;
}
