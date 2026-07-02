# Ticket 003 - Serializer

## Goal

Add the compiler-side export path that writes compiled module state to a
serialized artifact.

## Notes

The serializer should be a thin export layer over the existing compile path.
Spin up a normal `WrenVM`, compile the source the usual way, then write out the
compiled module state. Avoid creating a separate compiler pipeline for v1.

## Decisions Made

- The boundary between "core module variables" and "this module's own
  variables" must be captured by snapshotting
  `module->variableNames.count` immediately after the core-variable copy
  loop (mirroring `compileInModule` in `wren_vm.c`) and before `wrenCompile`
  is called. This is the only correct point to take the snapshot — taking
  it any later risks the user's own top-level `var` declarations already
  being mixed in with no way to tell them apart from core variables.
- `import "modulename" for X, Y` requires no special handling and is not an
  "unsupported construct." The compiler emits identical `CODE_IMPORT_MODULE`
  / `CODE_IMPORT_VARIABLE` opcodes with plain string constants regardless of
  whether the target is a user file, a host-provided module, or a compiled-in
  optional module (`random`, `meta`). The core/external distinction is
  resolved entirely at runtime in `importModule()` (`wren_vm.c`), not at
  compile time. So importing `random`/`meta`-style built-ins from serialized
  code works with zero extra serializer logic — the ticket's "reject
  unsupported output" criterion is not about banning import statements.
- There is no actual list of language constructs that produce
  unserializable `ObjFn` content. Constants are exhaustively one of: `null`,
  `true`/`false`, number, string, or nested `ObjFn` (verified by tracing
  every `addConstant`/`emitConstant` call site, including class attribute
  values and import name/module strings). `CODE_CLOSURE` upvalue metadata
  lives inline in the code byte stream, not in a side table. Foreign
  class/method declarations compile to ordinary opcodes; the actual C
  allocate/finalize bindings are resolved later at execution time and never
  touch the `ObjFn` tree. So "fails cleanly if compiled output contains
  something v1 does not support" should be implemented as a defensive
  fail-fast (e.g. an unreachable/default case in the constant-tag switch),
  not as a feature-detection pass. This guards against future compiler
  changes adding a new constant kind, not against any construct that exists
  today.
- **Byte width/endianness**: mirror what the compiler already does for
  in-bytecode operands — `emitShort` (`wren_compiler.c:1337`) writes 16-bit
  operands big-endian, high byte first. Use big-endian for every multi-byte
  integer field in the artifact (counts, lengths, the `double` constant
  payload) for consistency with that existing convention. Counts (variable
  count, constant-table count, code length) are written as 32-bit unsigned
  big-endian, since none of Wren's own buffers (`ByteBuffer`, `ValueBuffer`,
  `SymbolTable`) are indexed with anything narrower than `int`/`uint32_t`.
  Doubles are written as the raw 8-byte IEEE-754 bit pattern, byte-swapped to
  big-endian on little-endian hosts. This is explicitly not a portable
  cross-architecture ABI (see `bytecode-serialize.md`) — big-endian is
  chosen only for a single, unambiguous convention, not for portability
  guarantees beyond what v1 promises.
- **String encoding**: every string in the artifact (module variable names,
  string constants, debug function names) is written as a 32-bit big-endian
  byte length followed by that many raw bytes — no NUL terminator, no
  implicit encoding/charset conversion. This matches `ObjString`
  (`wren_value.h:152`), which stores `length` explicitly and treats `value`
  as an arbitrary byte buffer rather than a NUL-safe C string. Length-
  prefixing (instead of NUL-termination) is required because Wren string
  constants can legally contain embedded NUL bytes (e.g. via `\x00` escapes
  or byte-string literals).
- **Bool constant tag encoding**: use two distinct single-byte tags,
  `CONST_TRUE`/`CONST_FALSE`, alongside `CONST_NULL`/`CONST_NUM`/
  `CONST_STRING`/`CONST_FN` — six tag values total, no separate payload byte
  for booleans. This keeps every constant-table entry's shape uniform
  (one tag byte, optionally followed by a fixed-shape payload) and avoids a
  variable-shape "bool tag + separate value byte" case that the reader would
  otherwise need to special-case.
- **Debug info layout**: for each `ObjFn` (root and nested), write the
  function name as a length-prefixed string (per the string encoding above;
  `FnDebug->name`, `wren_value.h:212`), followed by the source-line buffer
  as a 32-bit big-endian count followed by that many 32-bit big-endian line
  numbers (`FnDebug->sourceLines`, an `IntBuffer` with exactly one entry per
  bytecode byte — see `emitByte`, `wren_compiler.c:1312`, which writes one
  line number per code byte in lockstep). The line-number entry count will
  therefore always equal that `ObjFn`'s code length; the loader can use this
  as a cheap consistency check when the debug-info flag is set.

## Breakdown

### 1. Capture the module-variable boundary

- Create the module the same way `compileInModule` does: create a fresh
  `ObjModule`, copy the loading VM's core module variables into it first.
- Immediately after that copy loop, record `variableNameBoundary =
  module->variableNames.count`. This is the index where the module's own
  user-declared variables begin.
- Compile the source into that module with the existing `wrenCompile` path.
- After compilation, the names to serialize are
  `module->variableNames[variableNameBoundary .. count)`.

### 2. Write the header

- Magic bytes `WREN` (per ticket 002).
- Version stamp: major/minor/patch bytes.
- Header flags: debug-info-present flag (v1 always sets this if debug info
  is written; see step 5).

### 3. Write module metadata

- Write the count and names of the module's own user-declared top-level
  variables (the slice identified in step 1).
- Do not write the module name (supplied by the loader's caller) and do not
  write core-module variable names/values.

### 4. Serialize the `ObjFn` tree

- Walk the root `ObjFn` recursively:
  - Write `code` (raw bytecode bytes, verbatim — this already includes
    inline `CODE_CLOSURE` upvalue metadata bytes).
  - Write the constant table as a 32-bit big-endian count followed by that
    many entries. For each constant, write a single tag byte followed by
    its payload:
    - `CONST_NULL` — no payload.
    - `CONST_TRUE` / `CONST_FALSE` — no payload (distinct tags, not a bool
      tag + value byte).
    - `CONST_NUM` — 8-byte big-endian IEEE-754 double.
    - `CONST_STRING` — length-prefixed string (32-bit big-endian length +
      raw bytes; see string encoding decision above).
    - `CONST_FN` — recurse into this same routine for the nested `ObjFn`.
  - Write `arity`, `numUpvalues`, `maxSlots` (single bytes — all three are
    bounded well under 256 by existing compiler limits).
  - Add a `default: fail` case to the constant-tag switch so an
    unrecognized constant `Value` type aborts the export cleanly instead of
    silently writing corrupt data (this is the "reject unsupported output"
    safety net called for in the acceptance criteria).

### 5. Write optional debug info

- Per ticket 002, debug info is in scope for v1. For the root and every
  nested `ObjFn`, gated behind the debug-info header flag from step 2,
  write:
  - the function name as a length-prefixed string (`FnDebug->name`)
  - the source-line buffer as a 32-bit big-endian count followed by that
    many 32-bit big-endian line numbers (`FnDebug->sourceLines`) — this
    count must equal that function's code length, since the compiler writes
    exactly one line number per code byte

### 6. Wire up the entry point

- Provide a single export function taking compiled module state (or source
  + module name, internally driving a normal `WrenVM` through steps 1-5)
  and producing bytes, writable to disk or returned in memory.
- The entry point should not require any change to the normal
  `wrenInterpret`/`compileInModule` path — it should observe/reuse that
  path's output, not fork it.

## Decision Checklist

- [x] Confirm where the core/user variable boundary is captured.
- [x] Confirm `import` statements targeting core-lib-alike optional modules
      need no special serializer handling.
- [x] Confirm there is no separate "unsupported construct" list to enforce
      beyond a defensive default case on the constant tag.
- [x] Decide byte width/endianness for counts and doubles: 32-bit big-endian
      counts/lengths, 8-byte big-endian IEEE-754 doubles, matching the
      existing big-endian convention `emitShort` already uses for in-code
      operands.
- [x] Decide string encoding: 32-bit big-endian length + raw bytes, no NUL
      terminator, to correctly handle strings with embedded NUL bytes.
- [x] Decide the exact on-disk bool constant tag encoding: dedicated
      `CONST_TRUE`/`CONST_FALSE` tags, no separate payload byte.
- [x] Decide the exact on-disk debug-info layout: length-prefixed function
      name string + big-endian count-prefixed line-number array, one line
      number per code byte.

## Acceptance Criteria

- A serializer entry point exists.
- It can write a compiled single-file module to disk or memory.
- It reuses the existing compiler/VM path rather than inventing a new IR.
- It snapshots the core/user module-variable boundary at the correct point
  (right after core variables are copied into the new module, before
  compiling) and serializes only the user-declared slice.
- All multi-byte integers (counts, lengths) are 32-bit big-endian; all
  doubles are 8-byte big-endian IEEE-754; all strings are length-prefixed
  raw bytes with no NUL terminator and no assumption of NUL-free content.
- The constant-table writer has a fail-fast default case so an unrecognized
  constant kind aborts the export instead of producing a corrupt artifact.
- Boolean constants use dedicated `CONST_TRUE`/`CONST_FALSE` tags rather than
  a shared bool tag plus payload byte.
- Debug info is serialized when present, gated by the header flag, using the
  length-prefixed-name + count-prefixed-line-array layout.
- It writes a recognizable artifact header (magic, version stamp, flags).
