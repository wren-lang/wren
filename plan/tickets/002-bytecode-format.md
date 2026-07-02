# Ticket 002 - Bytecode Format

## Goal

Define the minimal `.wrenc` artifact layout needed for the first-pass loader.

## Notes

Keep this format close to the compiled module shape the VM already produces.
Do not add fields just because they might be useful in a future multi-module or
portable format. If the loader does not need it for v1, leave it out.

## Decisions Made

- Magic bytes: `WREN`.
- Version stamp: one byte each for major, minor, and patch.
- Header flags: keep a debug-info flag for v1.
- Debug info: included in v1.
- V1 excludes external imports, bundles, obfuscation, and any stable ABI
  promise.
- The format should stay as close as practical to the compiled module shape
  already produced by the VM.
- The payload is the root `ObjFn` tree plus one small piece of module
  metadata: the names of the module's own user-declared top-level variables.
  The module name itself is not stored; it is supplied by the caller at load
  time, the same way `wrenInterpret(vm, module, source)` takes a module name
  as an argument rather than reading it from the source.
- Core-module variables (`System`, `Object`, `Fn`, `List`, `Map`, etc.) are
  never stored in the artifact. The loader must reconstruct them by copying
  the loading VM's own core module variables into the new module before
  wiring up the deserialized `ObjFn`, exactly as `compileInModule` does for a
  normal compile. See `bytecode-objfn-tree.md` for why: bytecode slot
  indices for `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` are positional, and the
  compiling VM's core module occupies the first N slots before any
  user-declared variable is added.

## Breakdown

### 1. Define the header

- Pick the magic bytes.
- Record the Wren version stamp.
- Decide whether the header needs any flags in v1.

### 2. Define the payload boundary

- Decide what minimal module metadata must be stored: the root `ObjFn` tree
  plus the names of the module's own user-declared top-level variables.
  Core-module variables are excluded and reconstructed live by the loader.
- Decide how the compiled function tree is represented.
- Decide whether optional debug data is part of v1.
- Note the constant table can hold `null`, booleans, numbers, strings, and
  nested `ObjFn`s (e.g. boolean literals can reach a constant table via
  class/method attribute values, not just `literal()` for numbers/strings).
  Any constant-tag encoding must account for all five, not just three.

### 3. Decide what is intentionally omitted

- No import graph.
- No bundle format.
- No external-module metadata.
- No stable ABI fields.

## Decision Checklist

- [x] Choose the file magic bytes.
- [x] Choose the Wren version stamp strategy.
- [x] Decide whether v1 needs header flags.
- [x] Decide whether the payload includes only the root compiled function or a
      small amount of module metadata as well.
- [x] Decide whether debug info is part of v1.
- [x] Decide whether the format should mirror current compiler internals very
      closely or introduce a small amount of normalization.
- [x] Confirm that external imports and module bundles are out of scope.

## Acceptance Criteria

- Header fields are defined.
- Version handling is defined.
- The payload shape is defined at a high level.
- The format stays close to the compiled module shape the VM already has.
- It is clear which metadata is required and which is intentionally omitted.
- The format does not assume user-defined imports or bundles.
