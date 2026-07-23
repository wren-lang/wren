# Testing Plan for Bytecode Serialization

## Existing Test Infrastructure

Wren has two C test layers we slot directly into — no new framework needed.

### Layer 1 — `test/unit/` (pure C unit tests)

Minimal `pass()` / `fail()` framework in `test.c` / `test.h`. Entry point is
`test/unit/main.c` which calls each test suite by name. Currently only
`path_test.c` exists. Adding a new suite means:

1. Create `test/unit/serialize_test.c` + `serialize_test.h`
2. Add `#include "serialize_test.h"` and `testSerialize()` to `main.c`

### Layer 2 — `test/api/` (C + Wren integration tests)

Each test is a `.c` file providing foreign method implementations paired with
a `.wren` file that drives the test via `System.print` output assertions. The
`api_tests.c` dispatcher routes foreign calls to the right C file.

Adding a new API test means:
1. Create `test/api/bytecode.c` + `bytecode.h` + `bytecode.wren`
2. Register foreign method bindings in `api_tests.c`

---

## Unit Tests — `test/unit/serialize_test.c`

Tests the serialization round-trip at the C level, without running any Wren
bytecode. Spin up a minimal VM, build or compile a small `ObjFn`, serialize
it, deserialize it into a fresh VM, and assert structural equality.

### Round-trip correctness

```
testRoundTrip_numConstant
  Compile: "1 + 2"
  Serialize → deserialize into fresh VM
  Assert: constants[0] == NUM_VAL(1), constants[1] == NUM_VAL(2)

testRoundTrip_stringConstant
  Compile: "\"hello\""
  Assert: constants[0] is ObjString "hello"

testRoundTrip_nestedFn
  Compile: "Fn.new { 42 }"
  Assert: constants[0] is ObjFn, its constants[0] == NUM_VAL(42)

testRoundTrip_closureUpvalue
  Compile: "var x = 1\nFn.new { x }"
  Assert: nested ObjFn has numUpvalues == 1
```

### Symbol remapping

```
testSymbolRemap
  VM-A: compile "System.print(\"hi\")"
        → CALL_1 encodes symbol index N in VM-A
  Serialize from VM-A
  VM-B: load a different set of modules first (shifts methodNames indices)
  Deserialize into VM-B
  Assert: CALL_1 arg in loaded bytecode == index of "print(_)" in VM-B
          (not the original N from VM-A)

testSymbolRemap_multipleSymbols
  Compile code that calls several distinct methods
  Assert all are correctly remapped independently
```

### Error handling

```
testBadMagic
  Write garbage header → deserialize returns NULL

testVersionMismatch
  Write header with wrong minor version → deserialize returns NULL

testTruncatedBuffer
  Write valid header then cut the buffer short at various offsets
  → deserialize returns NULL without crashing or overreading

testUnknownConstantTag
  Write a valid ObjFn with an unknown TAG byte in constants
  → deserialize returns NULL
```

### Obfuscation (if implemented)

```
testObfuscation_nameTablesUnreadable
  Serialize with key != 0
  Assert raw bytes of name table contain no plain-text method names

testObfuscation_roundTrip
  Serialize with key K → deserialize with key K → bytecode executes correctly

testObfuscation_wrongKey
  Serialize with key K → deserialize with key K+1
  → symbol names corrupt → remap fails or produces garbage symbol indices
```

---

## Integration Tests — `test/api/bytecode.wren` + `bytecode.c`

Tests the full pipeline end-to-end: Wren source → compile → serialize →
deserialize into fresh VM → execute → observe output.

The C side exposes foreign methods that drive the serialize/deserialize steps.
The Wren side calls them and prints expected output, which the test runner
diffs against a `.expect` file (same pattern as existing API tests).

### Basic execution

```wren
// Compile, serialize, deserialize, run — output must match source execution
var result = Bytecode.compileAndRun("System.print(\"hello\")")
System.print(result)  // expect: hello
```

### Classes and methods

```wren
var src = """
class Greeter {
  greet(name) { System.print("Hello, " + name) }
}
var g = Greeter.new()
g.greet("world")
"""
Bytecode.compileSerializeAndRun(src)
// expect: Hello, world
```

### Closures and upvalues

```wren
var src = """
var x = 10
var fn = Fn.new { System.print(x) }
fn.call()
"""
Bytecode.compileSerializeAndRun(src)
// expect: 10
```

### Module imports survive serialization

```wren
// Serialize a module that imports another
// Verify IMPORT_MODULE constant (string name) round-trips correctly
Bytecode.compileSerializeAndRun("import \"random\"\nSystem.print(\"ok\")")
// expect: ok
```

### Fresh VM isolation

```wren
// Deserialize into a VM that has loaded modules in a different order
// Verify method symbol remapping produces correct output
Bytecode.compileInVmA_runInVmB("System.print(1 + 2)")
// expect: 3
```

### Version rejection

```wren
// Attempt to load bytecode with wrong version header
var ok = Bytecode.tryLoadBadVersion()
System.print(ok)
// expect: false
```

---

## File Checklist

| File | Purpose |
|---|---|
| `test/unit/serialize_test.c` | Unit round-trip + remap + error tests |
| `test/unit/serialize_test.h` | Header |
| `test/unit/main.c` | Add `testSerialize()` call |
| `test/api/bytecode.c` | Foreign methods for integration tests |
| `test/api/bytecode.h` | Header |
| `test/api/bytecode.wren` | Wren-side test driver |
| `test/api/api_tests.c` | Register `bytecodeBindMethod` |
