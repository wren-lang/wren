# Ticket 003 Serializer — Implementation Notes

## What was built

A compiler-side bytecode export path for single-file Wren modules.

- New source: `src/vm/wren_serialize.c`
- Public API added to: `src/include/wren.h`
  - `WrenSerializeResult wrenSerializeModule(WrenConfiguration*, const char* module, const char* source, bool debugInfo)`
  - `void wrenFreeSerializeResult(WrenConfiguration*, WrenSerializeResult)`

## Design decisions

- **Reuse the normal VM + compiler path.** `wrenSerializeModule` creates a
  temporary `WrenVM`, builds a fresh `ObjModule`, copies in the core-module
  variables exactly as `compileInModule` does, snapshots
  `module->variableNames.count` immediately after the copy, then calls the
  unmodified `wrenCompile(vm, moduleObj, source, false, true)`.

- **Module name argument is intentionally ignored for v1.** The artifact
  carries no module name; the loader's caller supplies the name at load time.
  The temporary module is created with `wrenNewModule(vm, NULL)` and is not
  registered in `vm->modules`.

- **Variable boundary snapshot happens before compilation.** This is the only
  point where core variables and user variables can be told apart, because
  `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` use plain positional indices.

- **Constant-table tags (single byte each):**
  - `CONSTANT_NULL`    (0)
  - `CONSTANT_FALSE`   (1)
  - `CONSTANT_TRUE`    (2)
  - `CONSTANT_NUM`     (3) — followed by 8-byte big-endian IEEE-754 double
  - `CONSTANT_STRING`  (4) — followed by 32-bit big-endian length + raw bytes
  - `CONSTANT_FN`      (5) — recurse into nested `ObjFn`

- **Endianness / widths.** All counts and lengths are 32-bit big-endian;
  doubles are written as raw 64-bit bits in big-endian byte order; strings are
  length-prefixed with no NUL terminator. This mirrors the compiler's existing
  `emitShort` big-endian convention.

- **Debug info.** When `debugInfo` is true, for each `ObjFn` we write the
  function name as a length-prefixed string, then a 32-bit big-endian count
  of source-line entries followed by that many 32-bit big-endian line
  numbers. The header flags byte has bit 0 set whenever debug info is
  emitted.

- **Fail-fast for unsupported constants.** The default branch in
  `serializeConstant` sets `serializer->ok = false`. With current Wren this
  is purely defensive; the only constants produced today are null, bool,
  number, string, and nested fn.

- **Memory ownership.** The returned `bytes` are allocated with the same
  `reallocateFn` from the passed-in configuration (or the default allocator if
  none was supplied), so `wrenFreeSerializeResult` uses that same allocator.
  The temporary VM is destroyed after the bytes are copied out.

## Build-system changes

- Added `wren_serialize.o` to the object lists and file rules in all six GNU
  make variants:
  - `projects/make/wren.make`
  - `projects/make/wren_shared.make`
  - `projects/make.bsd/wren.make`
  - `projects/make.bsd/wren_shared.make`
  - `projects/make.mac/wren.make`
  - `projects/make.mac/wren_shared.make`
- `projects/premake/premake5.lua` already globs `../../src/**.c`, so a fresh
  premake run will pick up the new file automatically.
- VS/Xcode project files under `projects/vs2017`, `projects/vs2019`, and
  `projects/xcode` were not hand-updated. On those platforms run
  `util/generate_projects.py` (requires a `premake5` binary).

## Validation performed

- Built library and test targets on macOS:
  ```
  make -C projects/make.mac clean
  make -C projects/make.mac wren wren_shared wren_test
  ```
- Ran the existing test suite: `python3 util/test.py` exited 0.
- Wrote a one-off C harness that serialized sample source and verified the
  magic header (`WREN`), version stamp (`00 04 00`), debug flag, user
  variable count, and user variable names. Inspected the resulting artifact
  with `xxd`; layout matched the spec.

## Known caveats / future work

- No regression test was added under `test/api` in this changeset, to keep the
  diff minimal. If serialization becomes a first-class supported feature,
  consider a small API test covering success, compile-error failure, and the
  unsupported-constant fallback.
- There is no CLI convenience function that writes directly to disk; callers
  receive a memory buffer and write it themselves. This is sufficient for v1.
- The `module` name argument to `wrenSerializeModule` is currently only a
  placeholder for the public shape; the serializer does not embed it in the
  artifact.
