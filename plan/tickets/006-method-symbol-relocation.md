# Ticket 006 - Method Symbol Relocation

## Status

Implemented.

- Method-name symbol table is serialized and relocated on load.
- `CALL_*`, `SUPER_*`, `METHOD_INSTANCE`, and `METHOD_STATIC` operands are
  patched before execution.
- New API tests prove host `wrenCall` works for loaded class methods, and that
  preexisting method symbols in the loading VM do not break the artifact.
- New malformed-artifact tests cover truncated/excessive method-name tables and
  out-of-range operands.
- Release suite: `All 867 tests passed`.
- ASan debug suite: `All 867 tests passed`.

## Goal

Make bytecode-loaded modules safe and correct when their methods are invoked
from host code through `wrenCall`, and when artifacts are loaded into VMs whose
method-symbol table already contains host/user method names.

The loader must no longer execute bytecode using raw method-symbol indices from
the serializer VM. It must serialize the method-symbol table and relocate every
method-symbol operand to the loading VM's `vm->methodNames` indices.

## Problem

Compiled Wren bytecode encodes method names as integer symbol indices into
`vm->methodNames`, not as strings. These indices appear in bytecode operands for
method calls, `super` calls, and method definitions.

During serialization today, those indices come from the temporary serializer VM.
During loading today, the loader copies code bytes verbatim into the caller's
VM. That is only accidentally safe for code that stays entirely inside the
loaded artifact, because a method definition and an internal call can both use
the same stale serialized index.

It fails when host code uses `wrenCall` after load:

1. The loaded class method is bound at serialized symbol index `N`.
2. The host calls `wrenMakeCallHandle(vm, "greet(_)")`.
3. `wrenMakeCallHandle` interns `"greet(_)"` in the loading VM and returns the
   loading VM's symbol index `M`.
4. If `M != N`, `wrenCall` dispatches to method slot `M` and misses the method
   bound at slot `N`.

Observed failure from an attempted API test:

```text
Greeter metaclass does not implement 'greet(_)'.
```

The same loaded module can call `Greeter.greet("from-wren")` internally because
the internal call opcode and `METHOD_STATIC` opcode both use the same serialized
index. That does not prove API-level method calls are correct.

## Why Ticket 005 Did Not Catch This

Ticket 005 added API coverage for `wrenCall`, but the implemented test calls an
exported `Fn` object through `call()`:

```wren
var greet = Fn.new { System.print("hi") }
```

That exercises the core `Fn.call()` method, whose symbol is already stable in
the core method table. It does not call a method defined by the loaded bytecode
module.

The missing case is calling a loaded class/static/instance method through the C
API:

```wren
class Greeter {
  construct new() {}
  static greet(name) { System.print("static " + name) }
  hello(name) { System.print("instance " + name) }
}
```

The host should be able to load the bytecode, `wrenGetVariable(vm, "module",
"Greeter", 0)`, create call handles for `greet(_)`, `new()`, and `hello(_)`,
and call those methods successfully.

## Required Design

### 1. Serialize The Method-Symbol Table

Extend the artifact format to include the serializer VM's `vm->methodNames`
symbol table.

Recommended layout, immediately after the header and before module variable
metadata:

```text
magic/version/flags
uint32 methodNameCount
repeat methodNameCount:
  uint32 length
  bytes[length]
uint32 ownVariableCount
repeat ownVariableCount:
  uint32 length
  bytes[length]
ObjFn tree
```

Rationale:

- The method-symbol map is needed before `ObjFn` code can be relocated.
- It is independent of module variable slots.
- Keeping it near the header makes it clear this is VM/runtime metadata, not a
  module variable table.

Serializer requirements:

- Write every entry in `vm->methodNames` in index order.
- Write exact strings, not only user-defined names.
- Validate `methodNameCount <= MAX_METHODS + 1` before writing if possible.
- Lengths must fit `uint32` and should not exceed `MAX_METHOD_SIGNATURE` unless
  existing VM method names can legitimately exceed that. If in doubt, use
  `INT_MAX` as the serialization bound and have the loader enforce the stricter
  method-name bound only if it matches existing compiler behavior.

### 2. Build A Method-Symbol Relocation Map During Load

The loader must read the serialized method-name table before reading the
function tree.

For each serialized method index `oldIndex`:

1. Read and validate the length-prefixed method name.
2. Intern it into the loading VM with `wrenSymbolTableEnsure(vm,
   &vm->methodNames, name, length)`.
3. Store the returned loading-VM index in `methodSymbolMap[oldIndex]`.

Validation requirements:

- Reject if the method-name table is truncated.
- Reject if `methodNameCount > MAX_METHODS + 1`.
- Reject empty method names unless an existing VM invariant proves empty method
  names are allowed.
- Reject lengths that exceed `INT_MAX` or any tighter method signature bound used
  by the compiler/runtime.
- Reject if `wrenSymbolTableEnsure` returns an index greater than `MAX_METHODS`
  or greater than `UINT16_MAX`, because bytecode stores method operands in two
  bytes.
- Keep the relocation map alive until every function in the tree has had its
  code patched. The map can be normal C heap memory allocated through the host
  allocator or `malloc`; ensure all load-error exits free it.

Important: do not try to force the loading VM's `vm->methodNames` table to match
the serializer VM by index. That fails once a VM already has user method names
from previous source or bytecode modules. The loader must remap artifact indices
to the loading VM's actual indices instead.

### 3. Relocate Method-Symbol Operands In Every Loaded Function

After a function's code and constants are loaded, patch method-symbol operands
in `fn->code.data` from serialized indices to loading-VM indices.

Patch these opcodes:

- `CODE_CALL_0` through `CODE_CALL_16`: first two operand bytes are a method
  symbol.
- `CODE_SUPER_0` through `CODE_SUPER_16`: first two operand bytes are a method
  symbol, followed by a two-byte superclass-constant operand that must not be
  remapped.
- `CODE_METHOD_INSTANCE`: two-byte method symbol operand.
- `CODE_METHOD_STATIC`: two-byte method symbol operand.

Do not patch unrelated two-byte operands such as constants, jumps, module
variables, imports, or closure constants.

Implementation guidance:

- Add a loader-local code walker in `src/vm/wren_serialize.c`.
- Use `wren_compiler.c`'s existing instruction-length logic around
  `wrenBindMethodCode` as the reference for how many operand bytes each opcode
  has.
- `CODE_CLOSURE` is variable length: two bytes for the function constant index,
  then two bytes per upvalue in the nested function. Because closure length
  depends on the nested `ObjFn`, run relocation after constants have been loaded
  enough for `fn->constants` to contain child functions.
- Reject malformed code if the walker would read beyond `fn->code.count`. This
  remains structural validation of code framing, not full bytecode verification.
- Reject if an operand's old method index is greater than or equal to
  `methodNameCount`.
- Reject if the mapped new method index exceeds `UINT16_MAX`/`MAX_METHODS`.
- Patch operands in-place using big-endian encoding to match existing bytecode
  operands.

Suggested helper shape:

```c
static bool relocateMethodSymbols(ObjFn* fn,
                                  const int* methodSymbolMap,
                                  int methodSymbolCount)
```

This helper should be called for every `ObjFn`, including nested function
constants. Either call it at the end of each `loadFunctionBody()` after constants
are available, or do a recursive post-load walk of the root function tree before
module registration/execution.

### 4. Preserve Current Loader Guarantees

The fix must preserve the ticket 004 guarantees:

- Module registration remains deferred until all structural validation and
  relocation succeeds.
- Failed relocation returns `WREN_RESULT_LOAD_ERROR` and reports
  `WREN_ERROR_LOAD` through `errorFn` when configured.
- Failed relocation must not register or poison the requested module name.
- Attach-before-fill GC discipline must remain intact.
- Debug-stripped artifacts must still synthesize safe debug info.
- Unknown flags and trailing bytes must still be rejected.

### 5. Update Malformed-Artifact Tests For The New Section

Ticket 005's malformed-artifact parser helpers assume the current layout. Once
the method-symbol table is inserted, update those helpers to skip/read it before
own-variable metadata.

Add rejection coverage for the new method-symbol section:

- Truncated method-name table count.
- Truncated method-name string length.
- Truncated method-name string bytes.
- Excessive `methodNameCount`.
- Empty method name, if rejected.
- A bytecode operand referencing a method-symbol index outside the serialized
  method-name table.

## Tests To Add

### 1. C API Calls Loaded Static And Instance Methods

Add a test in `test/api/bytecode_api.c` that loads bytecode for:

```wren
class Greeter {
  construct new() {}
  static greet(name) { System.print("static " + name) }
  hello(name) { System.print("instance " + name) }
}
```

Then from C:

1. `wrenGetVariable(vm, "classmod", "Greeter", 0)`.
2. Call static method with handle `greet(_)`; assert output `static hi\n`.
3. Call constructor with handle `new()`; the instance remains in slot 0.
4. Call instance method with handle `hello(_)`; assert output includes
   `instance there\n`.

This test must fail before relocation and pass after relocation.

### 2. Existing Method Names In The Loading VM Do Not Break Loaded Bytecode

Add a test that deliberately shifts the loading VM's method-symbol table before
loading bytecode.

Example:

1. Create a `TestContext`.
2. Before `wrenInterpretBytecode`, call `wrenMakeCallHandle(ctx.vm,
   "preexisting(_)")` or interpret a separate source module defining unrelated
   user methods.
3. Load bytecode for a class with methods not previously interned.
4. Call loaded static/instance methods through `wrenCall`.

This ensures the loader remaps old serializer indices to the loading VM's actual
indices instead of assuming fresh-VM index equality.

### 3. Internal Calls Still Work After Relocation

Keep or add source-vs-bytecode equivalence for:

```wren
class Base { greet { "base" } }
class Derived is Base { greet { "derived " + super.greet } }
System.print(Derived.new.greet)
```

This already exists from ticket 005, but ticket 006 should explicitly treat it
as coverage for relocated `SUPER_*` operands.

### 4. Method Definition Relocation

The static/instance API test covers `CODE_METHOD_STATIC` and
`CODE_METHOD_INSTANCE`: if method definitions are not relocated, `wrenCall`
will still miss them.

### 5. Method Call Relocation

Add or keep an internal source-vs-bytecode call to a user-defined method with at
least one argument to cover `CODE_CALL_N` relocation.

Example:

```wren
class C { f(x) { x + 1 } }
System.print(C.new.f(41))
```

## Acceptance Criteria

- The artifact includes a serialized method-name symbol table.
- The loader builds an old-index-to-new-index method-symbol relocation map.
- The loader patches method-symbol operands in `CALL_*`, `SUPER_*`,
  `METHOD_INSTANCE`, and `METHOD_STATIC` instructions before execution.
- Relocation handles nested functions and closure bytecode without consuming one
  temp root per nesting level.
- Malformed method-symbol tables and out-of-range method operands fail with
  `WREN_RESULT_LOAD_ERROR`/`WREN_ERROR_LOAD` before module registration.
- Existing ticket 004 and 005 tests still pass.
- New API tests prove `wrenCall` works against bytecode-loaded static and
  instance methods defined by the loaded module.
- New tests prove loading into a VM with preexisting user method symbols still
  works.
- Source-vs-bytecode equivalence still covers inheritance and `super` after
  relocation.
- Targeted suite passes:

```sh
python3 util/test.py api/bytecode_loader
```

- Full suite passes before final handoff:

```sh
python3 util/test.py
```

- ASan debug suite passes before final handoff using the existing local
  convention:

```sh
ASAN_OPTIONS=detect_leaks=0 python3 util/test.py --suffix _d
```

## Non-Goals

- Do not add a full bytecode verifier.
- Do not change the v1 single-file scope.
- Do not add cross-version bytecode compatibility.
- Do not support bundled external module graphs.
- Do not attempt to make stale artifacts from the pre-006 format load
  successfully; this artifact format remains version-locked/internal.

## Implementation Order

1. Add the method-name table to serialization.
2. Add loader parsing and relocation-map creation.
3. Add code-walker relocation for method-symbol operands.
4. Update malformed-artifact helper offsets/tests for the new section.
5. Add the failing API method-call test.
6. Add preexisting-method-symbol-table test.
7. Run the bytecode API subset.
8. Run full plain and ASan release gates.
