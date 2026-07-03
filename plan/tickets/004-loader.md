# Ticket 004 - Loader

## Goal

Add the VM-side load path that reconstructs a compiled module from a serialized
artifact and executes it.

## Notes

The loader should be conservative. It should validate the header and version,
rebuild the compiled module, and reject malformed or incompatible input early.
It should not attempt to recover from arbitrary broken artifacts.

`foreign class`/`foreign method` declarations compile to ordinary opcodes and
carry no C bindings in the artifact — the actual allocate/finalize/foreign
function pointers are resolved at execution time via the loading VM's
`bindForeignClassFn`/`bindForeignMethodFn` config callbacks, exactly as they
would be for a normal source compile. This is a pre-existing host-
configuration requirement, not something the loader needs to validate or
embed. If the loading host doesn't register matching bindings, a foreign
constructor/method call fails at that point with a normal runtime error, the
same as running the original source on a VM missing those bindings would —
**but only if the loaded module has a real name** (see the module-name
decision below; `bindForeignClass`/`bindMethod` dereference `module->name->value`
unconditionally, so a `NULL` module name would crash instead of producing that
clean runtime error).

## Decisions Made

- **Ground truth is the shipped serializer, not the format doc's summary.**
  `src/vm/wren_serialize.c` already exists and defines the real, exact byte
  layout: magic `WREN`, 3 version bytes (major/minor/patch), 1 flags byte
  (`HEADER_FLAG_DEBUG_INFO = 0x01`), then `uint32` count + that many
  length-prefixed own-variable-name strings, then the `ObjFn` tree (`uint32`
  code length + bytes, `uint32` constant count + tagged constants, 1 byte each
  for arity/numUpvalues/maxSlots, then optional debug info gated by the flag
  byte). Constant tags are `CONSTANT_NULL(0)`, `CONSTANT_FALSE(1)`,
  `CONSTANT_TRUE(2)`, `CONSTANT_NUM(3)`, `CONSTANT_STRING(4)`,
  `CONSTANT_FN(5)` — the loader's tag switch must match these exact values.
  The loader ticket should be written against this file, not re-derived from
  `bytecode-format.md`.
- **Module name: default to a fixed placeholder, not `NULL`.** The serializer
  builds its throwaway module with `wrenNewModule(vm, NULL)` and never writes
  a name into the artifact (confirmed: no name bytes anywhere in
  `wrenSerializeModule`). `bindForeignClass` (`wren_vm.c:572`) and `bindMethod`
  (`wren_vm.c:359`) both dereference `module->name->value` unconditionally —
  with no null check — whenever a `foreign class`/`foreign method` declaration
  executes, even one that never gets called. A `NULL`-named loaded module
  would crash (not error cleanly) the moment such a declaration's defining
  bytecode runs (`CODE_METHOD_STATIC`/`CODE_METHOD_INSTANCE`/
  `CODE_FOREIGN_CLASS`), which happens unconditionally at module-body
  execution time, before any foreign method is actually called. For v1, the
  loader gives every loaded module a fixed placeholder name (e.g. the literal
  string `"<loaded>"`) via `wrenNewModule(vm, AS_STRING(wrenNewString(vm,
  "<loaded>")))` rather than `NULL`, regardless of what name (if any) the
  host passes to the load entry point. This sidesteps the crash with the
  smallest possible change and keeps the loader from having to special-case
  `NULL` throughout. The load entry point still takes a module name argument
  from the caller (for symmetry with `wrenInterpret` and for use in error
  messages the loader itself raises before execution starts), but that name
  is used only for the loader's own diagnostics — it is not the name wired
  into the reconstructed `ObjModule`, and it does not need to be registered
  in `vm->modules` (see next point).
- **Do not register the loaded module in `vm->modules`.** Mirrors the
  serializer's own choice (its comment: "We deliberately do not register this
  module in the VM's module map"). V1 has no import graph, so there is no
  scenario where another script needs to `import` a loaded module by name.
  Skipping registration also avoids collisions if the loader is called
  multiple times with the same placeholder name.
- **Reuse the exact same core-variable-copy loop `compileInModule` and the
  serializer both use**: iterate the live core module's `variables`/
  `variableNames` in order and call `wrenDefineVariable` for each, before
  reserving any slots for the artifact's own variable names. This is not a
  new design — copy the loop verbatim (it already appears twice in the
  codebase, in `wren_vm.c:453-481` and `wren_serialize.c:203-209`) so loader
  and serializer never drift.
- **Reserve user-declared variable slots by calling `wrenDefineVariable` with
  `NULL_VAL`**, one per name read from the artifact, immediately after the
  core-copy loop and before the `ObjFn` tree is wired in. This is exactly what
  the compiler's `declareVariable`/`wrenDeclareVariable` path does for an
  implicit top-level reference, except the loader has real names up front and
  values are never known at load time — they get populated when the
  deserialized module's own `CODE_STORE_MODULE_VAR` bytecode runs, same as a
  normal compile. Using `wrenDefineVariable` (not a raw buffer push) keeps the
  loader's module in exactly the same shape `ObjModule.variableNames` /
  `ObjModule.variables` would be in after a real compile, which matters
  because nothing else in the VM ever constructs these buffers by hand.
- **Rebuilding the `ObjFn` tree is manual construction, not decoding into an
  existing struct.** There is no `wrenNewFunctionFromBytes` — the loader must
  call `wrenNewFunction(vm, module, maxSlots)` to get an empty `ObjFn`, then
  fill `fn->code` and `fn->constants` by writing to those `ByteBuffer`/
  `ValueBuffer` fields directly with `wrenByteBufferWrite`/
  `wrenValueBufferWrite` (there is no bulk "set contents" helper), then set
  `fn->arity`/`fn->numUpvalues` directly (both are plain `int` fields, no
  setter), and finally call `wrenFunctionBindName` for the debug name (this is
  the only field with a dedicated setter, because it owns a heap allocation).
  This mirrors what `wrenCompile`'s own `initCompiler`/`endCompiler` pair does
  incrementally during a real compile — the loader is just doing it all at
  once from already-known bytes instead of token-by-token.
- **GC-safety during tree construction requires explicit rooting at every
  recursion level, not just at the top.** `wrenPushRoot`/`wrenPopRoot` use a
  fixed 8-slot stack (`WREN_MAX_TEMP_ROOTS = 8`, `wren_vm.h:11`) shared by the
  whole VM, so a naive "push every `ObjFn` as I create it and pop only at the
  very end" strategy can overflow on a deeply nested closure tree (a function
  with a function with a function...). The loader must push exactly one
  root per stack frame of its own recursive descent and pop it before
  returning from that frame, the same discipline the compiler's own
  `endCompiler` follows: root the new `ObjFn`, recurse into
  nested constants (each nested `CONSTANT_FN` allocates and roots its own
  `ObjFn`, then pops it once added to the parent's constant table), then pop
  the current level's root only after every constant has been written.
  `wrenValueBufferWrite` itself does *not* root anything — the compiler's
  own `addConstant` (`wren_compiler.c:510-513`) explicitly pushes a root
  around its `wrenValueBufferWrite` call for exactly this reason, and the
  loader must do the same around every constant-table write. Any string
  constant (`ObjString` from `wrenNewStringLength`) needs the same
  push-immediately-after-allocate treatment before it's written into the
  constants buffer.
- **Reject-on-limits, mirroring the compiler's own bounds, not just "trust the
  file."** Even though v1 assumes artifacts came from a matching Wren build,
  a corrupted or hand-crafted file could claim an out-of-range value. The
  loader should sanity-check `numUpvalues`/`arity` against `MAX_UPVALUES`
  (256) / `MAX_PARAMETERS` (16) — both `wren_common.h`/`wren_compiler.c`
  constants the compiler itself enforces — and reject the artifact rather
  than build a fiber whose stack-slot math (`wrenNewFiber`'s
  `maxSlots + 1` sizing) or upvalue array (`ALLOCATE_FLEX(vm, ObjClosure,
  ObjUpvalue*, fn->numUpvalues)`) could be corrupted by a bogus count. This is
  a defensive input-validation step, not a new compiler-limit design — it
  reuses limits the compiler already has.
- **Execution follows the exact `wrenInterpret` pattern, reusing
  `wrenCallFunction`/`runInterpreter`, not a new execution path.** Once the
  root `ObjFn` is rebuilt: wrap it in a closure with `wrenNewClosure`, root
  the closure, create a fiber with `wrenNewFiber(vm, closure)`, and call
  `runInterpreter(vm, fiber)` (the same two calls `wrenInterpret` makes at
  `wren_vm.c:1517-1525`). No new interpreter entry point is needed; the loader
  only needs a way to hand the closure to the *existing* internal
  `wrenInterpret`-style call sequence. Because `runInterpreter` and
  `wrenCallFunction` are internal (`wren_vm.h`, not `wren.h`), the loader must
  live in `src/vm/` (e.g. `wren_serialize.c`, next to the exporter) rather
  than being purely a `wren.h`-level host-side utility — it needs the same
  VM-internals access the serializer already has.
- **The loader creates and owns its own `WrenVM`, exactly as
  `wrenSerializeModule` does, rather than loading into an arbitrary
  caller-supplied VM.** This keeps the v1 contract symmetric ("create a VM,
  do the compile-or-load step, get a result") and avoids having to reason
  about loading serialized bytecode into a VM that has already been running
  arbitrary other scripts (stale `vm->lastModule`, existing fiber state,
  etc). A future ticket could relax this if embedding into a long-lived host
  VM turns out to be needed, but that is out of scope for v1.
- **Byte decoding mirrors the serializer's encoding exactly, field for
  field, in the same order.** Every `writeUint32`/`writeDouble`/`writeString`
  call in `wren_serialize.c` has a corresponding read that consumes the same
  number of bytes in the same big-endian convention
  (`wrenDoubleFromBits`, `wren_math.h:20`, is the exact inverse of
  `wrenDoubleToBits` already used by the serializer). There is no
  independent "loader format spec" to design — the loader's read functions
  are the mechanical inverse of `wren_serialize.c`'s write functions, one for
  one.
- **Truncation must be checked before every multi-byte read, not just at the
  top of the file.** Because every count in the format (variable count,
  code length, constant count, line-number count) determines how many
  subsequent bytes must exist, a truncated file can fail partway through the
  `ObjFn` tree, not just at the header. The reader needs a single "do we have
  at least N bytes left" check used before every read, and must bail out
  (freeing any partially-built VM state) the first time that check fails,
  rather than reading past the end of the buffer.

## Breakdown

### 1. Validate the header

- Check the buffer is at least 8 bytes (magic + version + flags) before
  reading anything.
- Check magic bytes equal `WREN`.
- Check version bytes equal `WREN_VERSION_MAJOR`/`MINOR`/`PATCH` exactly (v1
  is version-locked, per ticket 001 — no forward/backward compatibility
  window, no "major matches is enough" leniency).
- Read the flags byte and record whether `HEADER_FLAG_DEBUG_INFO` is set;
  this determines whether debug info is expected later in the stream.
- Any failure here returns a clean error result; no VM or module has been
  created yet, so there is nothing to tear down.

### 2. Create the loader's own VM and module

- Call `wrenNewVM(configuration)`, mirroring the serializer's own throwaway-VM
  pattern (this also means `bindForeignClassFn`/`bindForeignMethodFn`/
  `resolveModuleFn` etc. on the passed-in `configuration` are honored exactly
  as they would be for `wrenInterpret`, since they end up on this same VM).
- Create a fresh `ObjModule` with a fixed placeholder name (not `NULL` — see
  Decisions Made), root it.
- Copy the loading VM's own live core-module variables into it, via the same
  loop `compileInModule`/`wrenSerializeModule` both use.

### 3. Read module metadata and reserve variable slots

- Read the `uint32` own-variable count.
- For each, read the length-prefixed name and call `wrenDefineVariable(vm,
  module, name, length, NULL_VAL, NULL)` to reserve the slot in the same
  order they were written — this is what makes the deserialized bytecode's
  `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` indices line up.
- Reject if `wrenDefineVariable` ever returns `-2` (too many module
  variables) — this can only happen with a corrupt/hostile artifact claiming
  an absurd count, since a real compile would have already rejected the
  source before serialization.

### 4. Rebuild the `ObjFn` tree

- Recursive reader mirroring `serializeFunction`'s writer, one field at a
  time, in the same order: code bytes, constant table (tag-dispatched, one
  case per `ConstantTag` value, default case rejects the artifact), arity/
  numUpvalues/maxSlots bytes, then debug info if the header flag was set.
- Validate `numUpvalues <= MAX_UPVALUES` and `arity <= MAX_PARAMETERS` before
  trusting them to size anything.
- Root each newly allocated `ObjFn`/`ObjString` immediately after allocation
  and pop it only after it has been safely attached to its parent's constant
  table (see the GC-safety decision above) — do not defer rooting to the end
  of the whole tree walk.
- If the debug-info flag is set, also validate that the read line-count
  equals the read code-length for that function (this is the cheap
  consistency check `bytecode-serialize.md` already calls out) and reject the
  artifact if they disagree.
- Any tag byte, count, or length that would require reading past the end of
  the remaining buffer aborts the load immediately.

### 5. Execute the loaded module

- Wrap the root `ObjFn` in a closure (`wrenNewClosure`), root it.
- Create a fiber (`wrenNewFiber(vm, closure)`).
- Call `runInterpreter(vm, fiber)` and return its `WrenInterpretResult`
  exactly as `wrenInterpret` does.
- The VM created in step 2 stays alive for the duration of execution (unlike
  the serializer's VM, which is freed immediately after export) since running
  code needs a live VM; ownership/lifetime of this VM belongs to whatever
  API shape wraps this (see Decision Checklist below — this is the one open
  question left for the entry-point signature).

## Decision Checklist

- [x] Confirm the loader is written against the actual shipped
      `wren_serialize.c` byte layout, not a re-derivation from
      `bytecode-format.md`.
- [x] Decide what module name a loaded module gets: a fixed placeholder
      string, never `NULL`, to avoid a crash in `bindForeignClass`/
      `bindMethod`.
- [x] Confirm the loaded module is not registered in `vm->modules`.
- [x] Confirm the core-variable-copy step reuses the existing loop verbatim
      rather than reinventing it.
- [x] Confirm user-declared variable slots are reserved via
      `wrenDefineVariable(..., NULL_VAL, ...)`, not a raw buffer append.
- [x] Confirm there is no bulk "construct ObjFn from bytes" helper — the
      loader must call `wrenNewFunction` then fill fields/buffers manually.
- [x] Decide the GC-rooting discipline for recursive `ObjFn`/`ObjString`
      construction: root-per-recursion-level, popped as soon as attached to
      the parent, not "root everything, pop once at the end."
- [x] Decide which compiler limits the loader re-validates defensively:
      `MAX_UPVALUES`, `MAX_PARAMETERS` (and by extension the debug
      line-count-equals-code-length check already specified in
      `bytecode-serialize.md`).
- [x] Confirm execution reuses `wrenCallFunction`/`runInterpreter` via the
      same two-call sequence `wrenInterpret` already uses, rather than a new
      execution entry point.
- [x] Confirm the loader creates and owns its own `WrenVM`, matching the
      serializer's throwaway-VM pattern, rather than loading into a
      caller-supplied existing VM.
- [ ] Decide the exact public entry-point signature (return type carrying
      both a `WrenInterpretResult` and the `WrenVM*` the caller now owns and
      must eventually `wrenFreeVM`, versus a callback-based shape) — this is
      the one piece intentionally left open for implementation, since it's
      an API ergonomics choice rather than a behavioral one.

## Acceptance Criteria

- A loader entry point exists, implemented in `src/vm/` alongside the
  serializer (it needs internal VM access `wren.h` alone doesn't expose).
- It validates magic bytes and rejects anything that isn't exactly `WREN`.
- It validates the version stamp and rejects anything that isn't an exact
  major/minor/patch match (no compatibility window).
- It checks remaining-buffer length before every multi-byte read anywhere in
  the format (header, module metadata, and every level of the `ObjFn` tree),
  and rejects a truncated artifact immediately rather than reading past the
  end of the buffer.
- It creates its own `WrenVM` and reconstructs the core-module variable
  slots using the same copy loop `compileInModule`/`wrenSerializeModule` use,
  so `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` indices line up.
- It reserves the module's own user-declared variable slots via
  `wrenDefineVariable` with `NULL_VAL`, in artifact order, before wiring in
  the deserialized `ObjFn` tree.
- The loaded module is given a fixed non-`NULL` placeholder name and is not
  registered in the VM's module map.
- It rebuilds the `ObjFn` tree recursively, validating constant tags against
  the exact `ConstantTag` values `wren_serialize.c` writes, and rejects any
  unrecognized tag.
- It defensively re-validates `numUpvalues`/`arity` against the compiler's
  own `MAX_UPVALUES`/`MAX_PARAMETERS` limits and rejects artifacts that
  exceed them.
- If the debug-info header flag is set, it reads function names and
  source-line tables and verifies each function's line count equals its
  code length, rejecting a mismatch.
- Newly allocated `ObjFn`/`ObjString` objects are rooted immediately after
  allocation and popped only once safely attached to their parent, at every
  level of the recursive tree build — not deferred to the end of the whole
  walk.
- It hands the reconstructed root function to the VM for execution using the
  same `wrenNewClosure` → `wrenNewFiber` → `runInterpreter` sequence
  `wrenInterpret` uses, without introducing a parallel execution path.
- It does not require the original source file.
- `foreign class`/`foreign method` declarations in the loaded module resolve
  their C bindings via the loading VM's normal `bindForeignClassFn`/
  `bindForeignMethodFn` config callbacks and fail with a normal runtime
  error (not a crash) if those bindings are missing.
