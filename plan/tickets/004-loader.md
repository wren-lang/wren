# Ticket 004 - Loader

## Status

Implemented and verified.

- Loader and `wrenInterpretBytecode` API are implemented in `src/vm/wren_serialize.c`.
- Serializer output format was corrected before the loader read it.
- All standard tests pass: `867 tests passed`.
- Debug build with AddressSanitizer (`-fsanitize=address`, leaks disabled) also passes the full suite.
- The only API test (`test/api/bytecode_loader.wren`) is green.

## Goal

Add the VM-side load path that reconstructs a compiled module from a serialized
artifact and executes it.

## Notes

The loader should be conservative about the artifact container and metadata. It
should validate the header and version, check lengths/counts before allocating
or reading, rebuild the compiled module, and reject structurally malformed or
incompatible input early. It is not a bytecode verifier in v1: once code bytes
have passed framing/metadata checks, the bytecode payload is trusted as output
from the matching serializer/compiler. Hostile bytecode with valid framing can
still violate VM interpreter assumptions; rejecting that class of input requires
a future bytecode-verifier task, not just loader construction.

V1 artifacts are intended to represent one source file with no bundled external
module graph. The loader does not scan bytecode to reject import opcodes in v1.
If a loaded artifact contains and executes `CODE_IMPORT_MODULE`/
`CODE_IMPORT_VARIABLE`, those opcodes follow the VM's existing import path and
may call the host's `loadModuleFn`; any imported source/module remains outside
the serialized artifact and outside v1 bytecode loading scope.

`foreign class`/`foreign method` declarations compile to ordinary opcodes and
carry no C bindings in the artifact — the actual allocate/finalize/foreign
function pointers are resolved at execution time via the loading VM's
`bindForeignClassFn`/`bindForeignMethodFn` config callbacks, exactly as they
would be for a normal source compile. This is a pre-existing host-
configuration requirement, not something the loader needs to validate or
embed. If the loading host doesn't register matching bindings, behavior follows
the VM's existing source-compiled behavior for the same missing binding. The
loader does not add a new foreign-binding validation layer, and guaranteeing
that every missing foreign class allocator becomes a clean runtime error is out
of scope for this ticket unless the VM's existing foreign-class behavior is
changed separately.

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

- **The loader targets the corrected serializer layout, not the current
  one-byte trailing function metadata.** `src/vm/wren_serialize.c` exists and
  defines the current byte layout, but its function metadata layout is a
  prerequisite defect for the loader: it writes `arity`/`numUpvalues`/
  `maxSlots` after code/constants and truncates each to one byte. Before the
  loader is implemented, ticket 003/serializer format must be corrected so
  each `ObjFn` begins with metadata that is available before allocation:
  `uint8 arity`, `uint16 numUpvalues`, `uint32 maxSlots`, then `uint32` code
  length + bytes, `uint32` constant count + tagged constants, then optional
  debug info gated by the header flag. The outer artifact layout remains magic
  `WREN`, 3 version bytes (major/minor/patch), 1 flags byte
  (`HEADER_FLAG_DEBUG_INFO = 0x01`), then `uint32` count + that many
  length-prefixed own-variable-name strings, then the corrected `ObjFn` tree.
  Constant tags remain `CONSTANT_NULL(0)`, `CONSTANT_FALSE(1)`,
  `CONSTANT_TRUE(2)`, `CONSTANT_NUM(3)`, `CONSTANT_STRING(4)`,
  `CONSTANT_FN(5)` — the loader's tag switch must match these exact values.
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
  `wrenInterpretBytecode(vm, module, bytes, length)` — not like
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
- **The loaded module is registered in `vm->modules` before successful return,
  so it behaves like a normal loaded module after execution starts.** An
  earlier draft of this ticket said not to register it, reasoning from the
  serializer's choice not to register its own temporary module — but that
  reasoning doesn't transfer. The serializer's module is discarded moments
  later; the loader's module is meant to behave like any other loaded module
  in the host's VM afterward (`wrenHasModule`, `wrenGetVariable`,
  `wrenHasVariable`, `wrenCall` against exported methods, etc. should all
  work against it the same way they would for a module the host compiled
  from source). Not registering it would make loaded modules behave
  differently from compiled ones for no reason. Unlike `compileInModule`, the
  loader should defer registration until all pre-execution artifact validation
  and `ObjFn` reconstruction has succeeded. The loader rejects already-loaded
  names up front, so registering a module before structural validation would
  make a failed load poison that name for future load attempts in the same VM.
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
  reject the load with a clean error if the name is already taken, preferably
  by creating and rooting the module-name `Value` once and using that same
  rooted value for the internal module lookup, module creation, and map
  registration. If the final file placement cannot access the internal lookup,
  `wrenHasModule(vm, module)` is still behaviorally correct, but the creation
  path must still root and pop the actual name/module values in LIFO order.
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
  for this fork is in scope, and it's a purely additive change. Existing
  handlers in the tree are not exhaustive (`test/main.c:50-51` uses explicit
  `if` checks, `example/embedding/main.c:48-53` uses a `switch` with no
  `default:`), so hosts and tests that care about the new result need to add
  explicit handling rather than relying on an existing case. So: add
  `WREN_ERROR_LOAD` to `WrenErrorType` and `WREN_RESULT_LOAD_ERROR` to
  `WrenInterpretResult`. `WREN_ERROR_LOAD` reports a bytecode artifact load or
  validation error before execution: the `module` argument is the requested
  module name when available, while `line` is not meaningful and is passed as
  `-1`. This is distinct from both `WREN_ERROR_COMPILE` (which promises a real
  source line) and the initial `WREN_ERROR_RUNTIME` event (which has no
  meaningful module or line).
  Every pre-execution rejection listed above calls `vm->config.errorFn(vm,
  WREN_ERROR_LOAD, module, -1, message)` if `errorFn` is set (mirroring
  `printError`'s own `if (parser->vm->config.errorFn == NULL) return;`
  guard, `wren_compiler.c:427`), then the loader returns
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
  and serializer never drift. If the loader file can access the internal
  module lookup, use `getModule(vm, NULL_VAL)` for the core module as
  `compileInModule` does; otherwise use an equivalent checked lookup of the
  core module from `vm->modules`.
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
  existing struct.** There is no `wrenNewFunctionFromBytes` — after reading and
  validating the corrected leading metadata, the loader must call
  `wrenNewFunction(vm, module, maxSlots)` to get an empty `ObjFn`, then fill
  `fn->code` and `fn->constants` by writing to those `ByteBuffer`/
  `ValueBuffer` fields directly with `wrenByteBufferWrite`/
  `wrenValueBufferWrite` (there is no bulk "set contents" helper), then set
  `fn->arity`/`fn->numUpvalues` directly (both are plain `int` fields, no
  setter), and finally call `wrenFunctionBindName` for the debug name (this is
  the only field with a dedicated setter, because it owns a heap allocation).
  This mirrors what `wrenCompile`'s own `initCompiler`/`endCompiler` pair does
  incrementally during a real compile — the loader is just doing it all at
  once from already-known bytes instead of token-by-token.
- **GC-safety during tree construction uses attach-before-fill, not one
  long-lived temp root per recursion level.** `wrenPushRoot`/`wrenPopRoot` use
  a fixed 8-slot stack (`WREN_MAX_TEMP_ROOTS = 8`, `wren_vm.h:11`) shared by
  the whole VM, so keeping every active ancestor `ObjFn` on the temp-root stack
  would overflow on deeply nested closure trees. The loader should keep the
  root `ObjFn` rooted for the whole tree rebuild, then attach each nested
  `CONSTANT_FN` to its already-rooted parent before recursively filling it:
  allocate the child `ObjFn`, push it as a temporary root, write it into the
  parent's constants table with `wrenValueBufferWrite`, pop the child root, and
  only then recurse to fill the child body. From that point on, the child is
  reachable through the rooted root-function -> parent-constants chain, so
  nesting depth does not consume one temp-root slot per level. The root `ObjFn`
  has no parent, so its temp root remains live until after `wrenNewClosure` has
  safely attached it to a closure. `wrenValueBufferWrite` itself does *not* root
  anything — the compiler's own `addConstant` (`wren_compiler.c:510-513`)
  explicitly pushes a root around its `wrenValueBufferWrite` call for exactly
  this reason. String constants (`ObjString` from `wrenNewStringLength`) are
  already fully built when allocated, but still need the same
  push-immediately-after-allocate, write-to-constants, pop treatment.
- **Reject-on-limits, mirroring the compiler's own bounds, not just "trust the
  file."** Even though v1 assumes artifacts came from a matching Wren build,
  a corrupted or hand-crafted file could claim an out-of-range value. The
  loader should sanity-check leading function metadata before allocating or
  executing an `ObjFn`: `arity <= MAX_PARAMETERS` (16), `numUpvalues` within
  the compiler's upvalue bound once that bound is made available to the loader
  or intentionally duplicated, `maxSlots > 0`, and `maxSlots >= arity + 1`.
  The rebuilt root function must also have `numUpvalues == 0`: unlike nested
  functions, it is wrapped directly by the loader rather than by a parent
  `CODE_CLOSURE` instruction, so there is no enclosing scope that could
  populate root closure upvalue slots.
  The current serializer's one-byte `numUpvalues <= MAX_UPVALUES` check would
  be vacuous because `MAX_UPVALUES` is 256 and a byte can never exceed 255;
  this is one reason the format must be corrected before loader implementation.
  These checks are defensive structure/metadata validation, not a complete
  bytecode verifier.
- **Serialized counts and lengths must fit the VM's `int`-based buffers and
  compiler limits before allocation or buffer writes.** The artifact uses
  `uint32` lengths/counts in several places, but Wren's buffer counts and
  capacities are signed `int`, and the compiler imposes limits that bytecode
  operands rely on. The loader must reject values that would overflow or exceed
  those invariants before allocating or writing: `codeLength <= INT_MAX`,
  `constantCount <= MAX_CONSTANTS`, string/debug-name lengths `<= INT_MAX`, own
  variable names non-empty and `<= MAX_VARIABLE_NAME`, `lineCount <= INT_MAX`,
  and every source-line value fitting in `int` before it is written to an
  `IntBuffer`.
- **The v1 loader validates structure, not bytecode semantics.** The loader
  checks magic/version/flags, serialized lengths and counts, constant tags,
  function metadata bounds, debug table consistency, and truncation/trailing
  bytes. It does not walk bytecode instructions to prove opcode validity,
  operand bounds, module-variable indices, closure upvalue metadata length,
  import absence, jump targets, or stack effects. The interpreter trusts
  compiler-produced bytecode today, and v1 preserves that trust boundary:
  artifacts are expected to come from the matching serializer/compiler and from
  source within v1's single-file/no-bundled-import scope. A future verifier
  ticket can tighten this if hostile artifacts or import-opcode rejection become
  in scope.
- **Debug info is optional in the artifact, but safe `FnDebug` state is not
  optional in the VM.** `wrenNewFunction` always allocates a `FnDebug` object,
  and runtime stack-trace reporting assumes each non-core function has a
  source-line entry for each byte of bytecode. If `HEADER_FLAG_DEBUG_INFO` is
  set, the loader reads the serialized function name and source-line table and
  validates that the line count equals the code length. If the flag is not set,
  the loader synthesizes fallback debug state at load time instead of reading
  anything from the artifact: bind a non-NULL fallback function name (for
  example `"(bytecode)"`) and fill `fn->debug->sourceLines` with exactly
  `codeLength` placeholder entries (use `0` unless implementation discovers an
  existing Wren convention for unknown line numbers). This keeps runtime errors
  and stack traces memory-safe for debug-stripped artifacts without changing
  the byte format.
- **Execution follows the exact `wrenInterpret` setup pattern, not a new
  execution path.** Once the root `ObjFn` is rebuilt, it is still rooted from
  the tree construction step. Call `wrenNewClosure(vm, rootFn)` while that root
  is live, then pop the root-function root after the closure has been created.
  The loader then hands the closure to a narrow internal VM helper implemented
  in `wren_vm.c` and declared in `wren_vm.h` (for example,
  `wrenRunClosure(vm, closure)`). That helper owns the normal interpreter-entry
  setup: root the closure, create a fiber with `wrenNewFiber(vm, closure)`, pop
  the closure root, set `vm->apiStack = NULL`, and call the file-static
  `runInterpreter(vm, fiber)`, matching `wrenInterpret`'s setup at
  `wren_vm.c:1517-1525`. This avoids exposing `runInterpreter` itself while
  still keeping bytecode parse/rebuild code with the serializer for v1.
- **Byte decoding mirrors the corrected serializer's encoding exactly, field for
  field, in the same order.** Every `writeUint32`/`writeDouble`/`writeString`
  call in `wren_serialize.c` has a corresponding read that consumes the same
  number of bytes in the same big-endian convention
  (`wrenDoubleFromBits`, `wren_math.h:20`, is the exact inverse of
  `wrenDoubleToBits` already used by the serializer). There is no
  independent "loader format spec" to design once ticket 003 has corrected
  the function metadata layout — the loader's read functions are the
  mechanical inverse of the corrected serializer write functions, one for one.
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
  this determines whether debug info is expected later in the stream. Reject
  the artifact if any unknown flag bit is set:
  `(flags & ~HEADER_FLAG_DEBUG_INFO) != 0`.
- Any failure here returns a clean error result; no VM or module has been
  created yet, so there is nothing to tear down.

### 2. Validate the module name and create the module in the caller's VM

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
- Check whether `module` is already loaded in the caller's VM using the rooted
  `nameValue` and the internal module lookup if available (or use
  `wrenHasModule(vm, module)` before this allocation if final file placement
  does not expose that lookup). Reject the load with a clean error if so,
  popping the name root before returning — see the "reject if already loaded"
  decision above.
- Create the module with the rooted name (`wrenNewModule(vm,
  AS_STRING(nameValue))`), then root the module. Do not pop the name root here:
  `wrenPopRoot()` is LIFO and would pop the module root, not the name root.
- Do not register the module yet. Keep both the name and module roots live
  through core-variable copy, user-variable reservation, and `ObjFn` tree
  reconstruction. If any pre-execution structural validation fails, pop all
  loader temp roots and return `WREN_RESULT_LOAD_ERROR`; because the module was
  never inserted into `vm->modules`, the name remains retryable and partially
  built `ObjFn`/`ObjString` objects become ordinary unreachable GC objects.
- Copy the VM's own live core-module variables into it, via the same loop
  `compileInModule`/`wrenSerializeModule` both use.

### 3. Read module metadata and reserve variable slots

- Read the `uint32` own-variable count.
- For each, read the length-prefixed name and call `wrenDefineVariable(vm,
  module, name, length, NULL_VAL, NULL)` to reserve the slot in the same
  order they were written — this is what makes the deserialized bytecode's
  `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` indices line up.
- Before defining each own-variable name, reject it if the serialized length is
  zero, exceeds `MAX_VARIABLE_NAME`, exceeds `INT_MAX`, or would read past the
  end of the buffer.
- Reject the artifact if `wrenDefineVariable` returns any negative value. In
  the expected loader path, `-1` means the artifact name duplicates another
  reserved name or collides with a copied core variable name, and `-2` means
  too many module variables. `-3` is not expected because the loader never calls
  `wrenDeclareVariable`, which is what creates the numeric implicit-declaration
  sentinel that triggers that path, but treating any negative return as a load
  error keeps the loader defensive.

### 4. Rebuild the `ObjFn` tree

- Recursive reader mirroring the corrected `serializeFunction` writer, one
  field at a time, in the same order: `uint8 arity`, `uint16 numUpvalues`,
  `uint32 maxSlots`, `uint32` code length + code bytes, `uint32` constant count
  + tagged constants (one case per `ConstantTag` value, default case rejects
  the artifact), then debug info if the header flag was set.
- Validate leading function metadata before allocating the function:
  `arity <= MAX_PARAMETERS`, `numUpvalues` within the compiler's upvalue bound,
  `maxSlots > 0`, and `maxSlots >= arity + 1`.
- Validate serialized counts and lengths before allocation or buffer writes:
  `codeLength <= INT_MAX`, `constantCount <= MAX_CONSTANTS`, string constant
  lengths `<= INT_MAX`, debug-name lengths `<= INT_MAX`, `lineCount <=
  INT_MAX`, and each source-line value fits in `int`.
- After rebuilding the root function, validate that `rootFn->numUpvalues == 0`.
  Nested functions may have upvalues populated by their enclosing function's
  `CODE_CLOSURE` bytecode, but the root closure is created directly by the
  loader and has no enclosing function.
- Keep the root `ObjFn` rooted for the entire tree rebuild. For nested
  `CONSTANT_FN` values, attach-before-fill: allocate the child `ObjFn`, root it
  only while writing it into the parent's constants table, pop that temporary
  root, then recurse to fill the child now that it is reachable through the
  rooted parent tree. For `ObjString` constants, root immediately after
  allocation and pop only after the string has been written into the constants
  table.
- If the debug-info flag is set, read the function name and source-line table,
  validate that the read line count equals the read code length for that
  function, and reject the artifact if they disagree.
- If the debug-info flag is not set, synthesize safe fallback debug state for
  that function: bind a non-NULL fallback name and write exactly `codeLength`
  placeholder entries to `fn->debug->sourceLines`.
- Any tag byte, count, or length that would require reading past the end of
  the remaining buffer aborts the load immediately.
- After the root function tree has been read, require the reader to be exactly
  at the end of the input buffer (`reader.offset == reader.length`). The
  serializer writes no trailer, so trailing bytes indicate an artifact the
  serializer could not have produced and should be rejected before module
  registration or execution.
- After all pre-execution artifact validation and `ObjFn` reconstruction has
  succeeded, register the module in `vm->modules`. Keep the name, module, and
  root-function roots live through the `wrenMapSet` call because map growth can
  allocate and trigger GC. After registration, the module/name roots are no
  longer needed, but they sit below the root-function root on the LIFO temp-root
  stack. Reorder without allocation: pop the root-function root, pop the module
  root, pop the name root, then immediately push the root-function root again.
  This is safe because no allocation occurs during the pop/pop/pop/push window,
  the registered module now reaches its name, and the re-pushed root function
  reaches the module through `fn->module`.

### 5. Execute the loaded module

- Keep the root `ObjFn`'s tree-construction root live while calling
  `wrenNewClosure(vm, rootFn)`, since closure allocation can trigger GC before
  `closure->fn` is assigned.
- Pop the root `ObjFn` root after the closure has been created; no allocation
  happens between this pop and the internal helper immediately rooting the
  closure.
- Call the internal VM helper (implemented in `wren_vm.c`, declared in
  `wren_vm.h`) that roots the closure, creates a fiber with
  `wrenNewFiber(vm, closure)`, pops the closure root, sets
  `vm->apiStack = NULL`, calls `runInterpreter(vm, fiber)`, and returns its
  `WrenInterpretResult`.
- The caller's VM (passed into the loader, not created by it) stays alive
  for as long as the caller keeps it alive — the loader has no VM lifetime
  of its own to manage, unlike the serializer, which frees its throwaway VM
  immediately after export.

## Decision Checklist

- [x] Confirm the loader is written against the corrected serializer byte
      layout, with ticket 003/format updated first to place widened function
      metadata before code/constants.
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
      matching `printError`) with `WREN_ERROR_LOAD`, the requested module name,
      and line `-1` (this type is documented as having a meaningful module name
      but no meaningful source line — not smuggled in as a compile-error line).
      Runtime failures in the loaded module's own bytecode use the
      completely normal `WREN_ERROR_RUNTIME`/`WREN_RESULT_RUNTIME_ERROR`
      path already produced by `runInterpreter`, unchanged.
- [x] Confirm the core-variable-copy step reuses the existing loop verbatim
      rather than reinventing it.
- [x] Confirm user-declared variable slots are reserved via
      `wrenDefineVariable(..., NULL_VAL, ...)`, not a raw buffer append, and
      that any negative return is rejected defensively.
- [x] Confirm there is no bulk "construct ObjFn from bytes" helper — the
      loader must read leading metadata, call `wrenNewFunction(vm, module,
      maxSlots)`, then fill fields/buffers manually.
- [x] Decide the GC-rooting discipline for recursive `ObjFn`/`ObjString`
      construction: attach nested `ObjFn`s to a rooted parent before filling
      them, keep the root `ObjFn` rooted through closure creation, and use
      short push/write/pop windows for child functions and string constants —
      not one long-lived temp root per recursion level and not "root
      everything, pop once at the end."
- [x] Decide which compiler limits the loader re-validates defensively:
      `MAX_PARAMETERS`, the compiler's upvalue bound once made accessible or
      intentionally duplicated, `maxSlots > 0`, `maxSlots >= arity + 1`, and
      the debug line-count-equals-code-length check already specified in
      `bytecode-serialize.md`.
- [x] Confirm execution reuses the same `wrenNewClosure` -> `wrenNewFiber` ->
      `vm->apiStack = NULL` -> `runInterpreter` setup pattern `wrenInterpret`
      uses, through a narrow internal VM helper rather than exposing
      `runInterpreter` or introducing a parallel execution path.
- [x] Confirm the loader takes the caller's existing `WrenVM*` as a
      parameter and does not create or own a VM — the loader is symmetric
      with `wrenInterpret`, not with `wrenSerializeModule`'s throwaway-VM
      pattern.
- [x] Decide the exact public entry-point signature:
      `WrenInterpretResult wrenInterpretBytecode(WrenVM* vm,
      const char* module, const uint8_t* bytes, size_t length)`, matching
      `wrenInterpret`'s shape with a byte buffer in place of a source string
      and avoiding confusion with the existing `WrenLoadModuleFn`/
      `loadModuleFn` import-source callback.

## Acceptance Criteria

- A loader entry point exists, implemented in `src/vm/` alongside the
  serializer (it needs internal VM access `wren.h` alone doesn't expose).
- Ticket 003/serializer format is corrected before loader implementation so
  each serialized `ObjFn` writes `uint8 arity`, `uint16 numUpvalues`, and
  `uint32 maxSlots` before code bytes and constants; the loader targets that
  corrected layout, not the current one-byte trailing metadata layout.
- It takes the caller's existing `WrenVM*` as a parameter and loads into it,
  the same way `wrenInterpret` compiles into an existing VM — it does not
  create or own a VM of its own.
- It validates magic bytes and rejects anything that isn't exactly `WREN`.
- It validates the version stamp and rejects anything that isn't an exact
  major/minor/patch match (no compatibility window).
- It rejects artifacts with any unknown header flag bit set; v1 only accepts
  `HEADER_FLAG_DEBUG_INFO`.
- It checks remaining-buffer length before every multi-byte read anywhere in
  the format (header, module metadata, and every level of the `ObjFn` tree),
  and rejects a truncated artifact immediately rather than reading past the
  end of the buffer.
- It rejects artifacts with trailing bytes after the root function tree; the
  reader must consume the entire buffer before module registration/execution.
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
  `WREN_RESULT_LOAD_ERROR` (added to `WrenInterpretResult`). `WREN_ERROR_LOAD`
  is documented as reporting the requested module name when available, with no
  meaningful source line (`line == -1`), rather than overloading
  `WREN_ERROR_COMPILE`'s line field with a sentinel or copying
  `WREN_ERROR_RUNTIME`'s no-module/no-line contract.
- The module name string is rooted before the module is created (not combined
  into a single unrooted expression), both the name and module roots stay live
  until the deferred `wrenMapSet`, and registration happens only after
  pre-execution validation/reconstruction succeeds. After registration, the
  loader reorders temp roots without allocation so the name/module roots are
  released while the root `ObjFn` remains rooted for closure creation.
- A structural load failure before execution does not leave the requested module
  name registered in `vm->modules`, so a host can retry the load in the same VM.
- It reconstructs the core-module variable slots using the same copy loop
  `compileInModule`/`wrenSerializeModule` use, so `LOAD_MODULE_VAR`/
  `STORE_MODULE_VAR` indices line up.
- It reserves the module's own user-declared variable slots via
  `wrenDefineVariable` with `NULL_VAL`, in artifact order, before wiring in
  the deserialized `ObjFn` tree, rejects empty or overlong variable names, and
  rejects the artifact on any negative return. Expected negative returns are
  duplicate/core-name collision (`-1`) and too many variables (`-2`); `-3` is
  not expected because the loader never creates implicit-declaration sentinels.
- The loaded module is given the caller's real, required module name (not a
  placeholder) and is registered in the VM's module map, the same as any
  normally-compiled module.
- It rebuilds the `ObjFn` tree recursively, validating constant tags against
  the exact `ConstantTag` values the corrected serializer writes, and rejects
  any unrecognized tag.
- It defensively validates leading function metadata before allocating each
  `ObjFn`: `arity <= MAX_PARAMETERS`, `numUpvalues` within the compiler's
  upvalue bound, `maxSlots > 0`, and `maxSlots >= arity + 1`.
- It validates all serialized counts and lengths before allocation or buffer
  writes: `codeLength <= INT_MAX`, `constantCount <= MAX_CONSTANTS`,
  string/debug-name lengths `<= INT_MAX`, own variable names non-empty and
  `<= MAX_VARIABLE_NAME`, `lineCount <= INT_MAX`, and source-line values fit in
  `int`.
- It rejects artifacts whose root function has `numUpvalues != 0`, because the
  root closure is created directly by the loader and has no enclosing function
  to populate upvalue slots.
- If the debug-info header flag is set, it reads function names and
  source-line tables and verifies each function's line count equals its
  code length, rejecting a mismatch.
- If the debug-info header flag is not set, it synthesizes safe fallback debug
  state for every function by binding a non-NULL fallback name and writing one
  placeholder source-line entry per byte of bytecode.
- The root `ObjFn` remains rooted for the whole tree rebuild and through
  closure creation. Nested `ObjFn`s use attach-before-fill: each child is rooted
  only while it is written into its parent's constants table, then recursively
  filled after it is reachable through the rooted parent tree. `ObjString`
  constants are rooted immediately after allocation and popped after they are
  written into the constants table.
- It hands the reconstructed root function to the VM for execution using the
  same `wrenNewClosure` -> `wrenNewFiber` -> `vm->apiStack = NULL` ->
  `runInterpreter` setup pattern `wrenInterpret` uses, with the root `ObjFn`
  rooted through closure creation and a narrow internal VM helper rooting the
  closure through fiber creation, without exposing `runInterpreter` or
  introducing a parallel execution path.
- It does not require the original source file.
- It does not scan bytecode to reject import opcodes in v1. Artifacts are
  expected to be generated from source within the single-file scope; if import
  opcodes execute anyway, they use the VM's existing host `loadModuleFn` path
  and any imported module is outside the serialized artifact.
- `foreign class`/`foreign method` declarations in the loaded module resolve
  their C bindings via the loading VM's normal `bindForeignClassFn`/
  `bindForeignMethodFn` config callbacks. Missing bindings follow the VM's
  existing source-compiled behavior; changing missing foreign-class allocator
  failures into guaranteed clean runtime errors is outside this loader ticket.
