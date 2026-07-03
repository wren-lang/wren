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
same as running the original source on a VM missing those bindings would.

The loader is symmetric with `wrenInterpret`, not with `wrenSerializeModule`.
It takes the host's own existing `WrenVM*` (the same one the host already
created with `wrenNewVM` and configured with its own `bindForeignClassFn`/
`writeFn`/etc.) and loads the artifact into that VM, the same way
`wrenInterpret(vm, module, source)` compiles source into it. It does not
create or own a VM of its own. This also means the caller supplies a real,
required module name (not a placeholder), so `bindForeignClass`/`bindMethod`
(`wren_vm.c:572`, `:359`), which dereference `module->name->value`
unconditionally, never see a `NULL` name.

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
- **The loader takes the caller's existing `WrenVM*` as a parameter; it does
  not create or own a VM.** This was the wrong call in an earlier draft of
  this ticket, which mirrored `wrenSerializeModule`'s throwaway-VM pattern.
  That pattern fits the serializer because its VM only exists to satisfy
  `wrenCompile`'s requirement that a module belong to some VM, and it's freed
  the instant the bytes are copied out — nothing outside that function ever
  touches it. The loader is different: its whole point is to hand a runnable
  module to the *host's* long-lived VM, the same VM the host built with
  `wrenNewVM` and its own `bindForeignClassFn`/`writeFn`/etc., and will keep
  using afterward (calling exported methods, inspecting variables, etc.).
  The loader's entry point should look like `wrenInterpret`'s signature —
  `wrenLoadModule(vm, module, bytes, length)` — not like
  `wrenSerializeModule`'s "spin up, do work, tear down" shape. This removes
  the earlier open checklist item about VM ownership: there is no VM
  lifetime question because the loader never allocates one.
- **The caller supplies a real, required module name — no placeholder.**
  Because the loader now runs inside the caller's real VM, the module name
  argument is not just a diagnostics string; it becomes the actual
  `ObjModule.name`, the same as the `module` argument to `wrenInterpret`. This
  removes the earlier `NULL`-name workaround entirely: `bindForeignClass`/
  `bindMethod` (`wren_vm.c:572`, `:359`) get a real name to dereference like
  any normally-compiled module, so no placeholder string is needed.
- **The loaded module is registered in `vm->modules`, exactly like a normal
  `wrenInterpret`/`compileInModule` call would register it.** An earlier
  draft of this ticket said not to register it, reasoning from the
  serializer's choice not to register its own temporary module — but that
  reasoning doesn't transfer. The serializer's module is discarded moments
  later; the loader's module is meant to behave like any other loaded module
  in the host's VM afterward (`wrenHasModule`, `wrenGetVariable`,
  `wrenHasVariable`, `wrenCall` against exported methods, etc. should all
  work against it the same way they would for a module the host compiled
  from source). Not registering it would make loaded modules behave
  differently from compiled ones for no reason.
- **New decision this correction surfaces: reject if the requested module
  name is already loaded.** Since the loader now shares the host's real
  `vm->modules` map, calling it with a name that's already registered is a
  new failure mode that didn't exist under the throwaway-VM design (where
  every load got its own private map). `compileInModule`'s existing behavior
  for an already-loaded module name is to reuse the existing `ObjModule` and
  compile new code into it (see `getModule`/`compileInModule`,
  `wren_vm.c:453-460`) — that behavior exists to support a script `import`ing
  the same module twice, not to support replacing an already-loaded module's
  contents. Silently reusing an existing module here would be wrong: the
  artifact's own variable-slot layout assumes it is populating a module from
  scratch, right after the core-variable copy, and an existing module could
  already have arbitrary other variables in those slots. The loader should
  check `wrenHasModule(vm, module)` (or the internal `getModule` equivalent)
  up front and reject the load with a clean error if the name is already
  taken, rather than attempting to merge into or overwrite an existing
  module.
- **How the loader reports its own structural failures to the host: add a
  dedicated `WREN_ERROR_LOAD`/`WREN_RESULT_LOAD_ERROR` pair, don't overload
  the compile-error ones.** A first pass at this ticket planned to reuse
  `WREN_ERROR_COMPILE`/`WREN_RESULT_COMPILE_ERROR` with a `-1` sentinel line
  number for artifact-structure failures (bad magic, wrong version,
  truncated buffer, unrecognized constant tag, mismatched debug line count,
  an already-loaded module name, an out-of-range `numUpvalues`/`arity`, a
  `wrenDefineVariable` failure). That's a poor fit: `wren.h`'s own
  `WrenErrorFn` doc comment says a `WREN_ERROR_COMPILE` call reports "the
  resolved name of the module **and line where the error occurs**" — a real
  promise about what that field means, which a malformed-artifact rejection
  can't honor because there's no source involved at all. `-1` isn't "the
  actual line," it's a lie of convenience. `WREN_ERROR_RUNTIME` avoids this
  same problem correctly, by being *documented* as having no line
  (`wren.h:124-126`) rather than reusing another type's line field with a
  sentinel. Since ticket 001 already establishes this artifact format is
  version-locked and not a stable public ABI, extending `wren.h`'s enums
  for this fork is in scope, and it's a purely additive change — neither
  existing `switch` over `WrenInterpretResult` in the tree
  (`test/main.c:50-51`, `example/embedding/main.c:48-53`) has a `default:`
  case, so adding a value doesn't silently misroute anything; it just means
  a host that wants to handle the new case must add one. So: add
  `WREN_ERROR_LOAD` to `WrenErrorType` (module resolved name, no meaningful
  line — pass `-1` the same way `WREN_ERROR_RUNTIME` already does, but
  documented as "no line" the same way that type is, not smuggled in as a
  compile-error line) and `WREN_RESULT_LOAD_ERROR` to `WrenInterpretResult`.
  Every pre-execution rejection listed above calls `vm->config.errorFn(vm,
  WREN_ERROR_LOAD, module, -1, message)` if `errorFn` is set (mirroring
  `printError`'s own `if (parser->vm->config.errorFn == NULL) return;`
  guard, `wren_compiler.c:426`), then the loader returns
  `WREN_RESULT_LOAD_ERROR`. Failures that occur *during* execution of the
  loaded module's own bytecode (a foreign call with no matching binding, an
  unhandled runtime error in the loaded code, etc.) go through the
  completely normal `WREN_ERROR_RUNTIME`/`WREN_ERROR_STACK_TRACE`/
  `WREN_RESULT_RUNTIME_ERROR` path `runInterpreter` already produces — the
  loader does nothing special there. This gives the loader three clean,
  distinguishable outcomes for a host to switch on: "artifact was rejected
  before anything ran" (`WREN_RESULT_LOAD_ERROR`), "the loaded code ran and
  hit a runtime error" (`WREN_RESULT_RUNTIME_ERROR`), and success.
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

### 2. Validate the module name and create the module in the caller's VM

- Check whether `module` is already loaded in the caller's VM (`getModule`/
  `wrenHasModule`). Reject the load with a clean error if so — see the
  "reject if already loaded" decision above.
- Create the module name string and root it *before* creating the module:
  `Value nameValue = wrenNewString(vm, module); wrenPushRoot(vm,
  AS_OBJ(nameValue));`. This ordering matters — `wrenNewModule`'s own
  `ALLOCATE(vm, ObjModule)` can trigger a GC, and at that point the name
  string isn't reachable from anywhere yet (not in `vm->modules`, not on any
  root stack) unless it was pushed first. This is the same ordering
  `defineClass` (`wren_core.c:1224-1225`) and `wrenHasModule`
  (`wren_vm.c:1970-1971`) already use for exactly this reason — do not
  combine the two calls into one expression
  (`wrenNewModule(vm, AS_STRING(wrenNewString(vm, module)))`), since that
  leaves the intermediate string unrooted for the duration of the outer
  call.
- Create the module with the rooted name (`wrenNewModule(vm,
  AS_STRING(nameValue))`), root the module, then pop the name string's root
  (the module now holds the only reference it needs via `module->name`).
- Register it in `vm->modules` immediately, the same point
  `compileInModule` registers its module (`wren_vm.c:465-469`) — i.e. before
  the rest of the artifact has been read or validated. This mirrors
  `compileInModule`'s existing behavior/tradeoff: if the load fails partway
  through (steps 3/4 below), the module stays in `vm->modules` in a
  partially-populated state, exactly as a module that fails to compile from
  source does today (see the existing `// TODO: Should we still store the
  module even if it didn't compile?` at `wren_vm.c:487`). This is a
  pre-existing behavior the loader inherits rather than a new tradeoff it
  introduces.
- Copy the VM's own live core-module variables into it, via the same loop
  `compileInModule`/`wrenSerializeModule` both use.

### 3. Read module metadata and reserve variable slots

- Read the `uint32` own-variable count.
- For each, read the length-prefixed name and call `wrenDefineVariable(vm,
  module, name, length, NULL_VAL, NULL)` to reserve the slot in the same
  order they were written — this is what makes the deserialized bytecode's
  `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` indices line up.
- Check the return value against both failure cases `wrenDefineVariable` can
  produce here, not just one: `-2` means too many module variables (an
  absurd count a real compile would have already rejected before
  serialization), and `-1` means the name was already defined in this
  module — which, this early in the load, can only mean the artifact lists
  the same own-variable name twice (a real compile could never have produced
  a duplicate entry in this slice of `variableNames`, since
  `wrenDefineVariable` itself is what enforces uniqueness at compile time).
  Reject the artifact on either return value; `-3` (the "local name
  referenced before definition" case) cannot occur here since every call
  passes `NULL_VAL`, never a number, so `line` is never written and that
  branch is unreachable from this call site.

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
- The caller's VM (passed into the loader, not created by it) stays alive
  for as long as the caller keeps it alive — the loader has no VM lifetime
  of its own to manage, unlike the serializer, which frees its throwaway VM
  immediately after export.

## Decision Checklist

- [x] Confirm the loader is written against the actual shipped
      `wren_serialize.c` byte layout, not a re-derivation from
      `bytecode-format.md`.
- [x] Decide what module name a loaded module gets: the caller's real,
      required module name (mirroring `wrenInterpret`'s `module` argument),
      not a placeholder — this is what a real `ObjModule.name` needs to
      avoid a crash in `bindForeignClass`/`bindMethod`.
- [x] Confirm the loaded module is registered in `vm->modules`, the same as
      any normally-compiled module.
- [x] Decide what happens if the caller's requested module name is already
      loaded in the VM: reject the load cleanly rather than reusing or
      overwriting the existing module.
- [x] Decide how the loader reports its own structural failures (bad magic,
      wrong version, truncation, name collision, bad tag, limit violation)
      to the host: add a dedicated `WREN_ERROR_LOAD`/`WREN_RESULT_LOAD_ERROR`
      pair to `wren.h` rather than overloading `WREN_ERROR_COMPILE`'s
      documented "line where the error occurs" field with a fake `-1`.
      Reports go through `vm->config.errorFn` (guarded by a NULL check,
      matching `printError`) with `WREN_ERROR_LOAD` and line `-1` (this type
      is documented as having no meaningful line, the same way
      `WREN_ERROR_RUNTIME` already is — not smuggled in as a compile-error
      line). Runtime failures in the loaded module's own bytecode use the
      completely normal `WREN_ERROR_RUNTIME`/`WREN_RESULT_RUNTIME_ERROR`
      path already produced by `runInterpreter`, unchanged.
- [x] Confirm the core-variable-copy step reuses the existing loop verbatim
      rather than reinventing it.
- [x] Confirm user-declared variable slots are reserved via
      `wrenDefineVariable(..., NULL_VAL, ...)`, not a raw buffer append, and
      that both its `-1` (duplicate name) and `-2` (too many variables)
      failure returns are checked, not just `-2`.
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
- [x] Confirm the loader takes the caller's existing `WrenVM*` as a
      parameter and does not create or own a VM — the loader is symmetric
      with `wrenInterpret`, not with `wrenSerializeModule`'s throwaway-VM
      pattern.
- [ ] Decide the exact public entry-point signature: most likely
      `WrenInterpretResult wrenLoadModule(WrenVM* vm, const char* module,
      const uint8_t* bytes, size_t length)`, matching `wrenInterpret`'s
      shape with a byte buffer in place of a source string. This is left
      open for implementation as an API ergonomics choice, not a behavioral
      one.

## Acceptance Criteria

- A loader entry point exists, implemented in `src/vm/` alongside the
  serializer (it needs internal VM access `wren.h` alone doesn't expose).
- It takes the caller's existing `WrenVM*` as a parameter and loads into it,
  the same way `wrenInterpret` compiles into an existing VM — it does not
  create or own a VM of its own.
- It validates magic bytes and rejects anything that isn't exactly `WREN`.
- It validates the version stamp and rejects anything that isn't an exact
  major/minor/patch match (no compatibility window).
- It checks remaining-buffer length before every multi-byte read anywhere in
  the format (header, module metadata, and every level of the `ObjFn` tree),
  and rejects a truncated artifact immediately rather than reading past the
  end of the buffer.
- It rejects the load cleanly if the caller-supplied module name is already
  loaded in the VM, rather than reusing or overwriting the existing module.
- Every structural rejection that happens before the loaded module's own
  bytecode runs (bad magic, wrong version, truncation, unrecognized
  constant tag, mismatched debug line count, name collision, an
  out-of-range `numUpvalues`/`arity`, a `wrenDefineVariable` failure)
  reports through `vm->config.errorFn` as the new `WREN_ERROR_LOAD` type
  (when `errorFn` is set) with line `-1`, and the loader returns the new
  `WREN_RESULT_LOAD_ERROR`. Failures during execution of the loaded
  module's own bytecode go through the normal `WREN_ERROR_RUNTIME`/
  `WREN_RESULT_RUNTIME_ERROR` path already produced by `runInterpreter`,
  with no loader-specific handling.
- `wren.h` gains `WREN_ERROR_LOAD` (added to `WrenErrorType`) and
  `WREN_RESULT_LOAD_ERROR` (added to `WrenInterpretResult`), both
  documented the same way `WREN_ERROR_RUNTIME` already documents having no
  meaningful line/module, rather than overloading `WREN_ERROR_COMPILE`'s
  line field with a sentinel.
- The module name string is rooted before the module is created (not
  combined into a single unrooted expression), so a GC triggered by
  `wrenNewModule`'s own allocation cannot collect it.
- It reconstructs the core-module variable slots using the same copy loop
  `compileInModule`/`wrenSerializeModule` use, so `LOAD_MODULE_VAR`/
  `STORE_MODULE_VAR` indices line up.
- It reserves the module's own user-declared variable slots via
  `wrenDefineVariable` with `NULL_VAL`, in artifact order, before wiring in
  the deserialized `ObjFn` tree, and rejects the artifact on either a `-1`
  (duplicate name within the artifact's own variable list) or `-2` (too
  many variables) return.
- The loaded module is given the caller's real, required module name (not a
  placeholder) and is registered in the VM's module map, the same as any
  normally-compiled module.
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
