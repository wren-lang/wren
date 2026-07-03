# Bytecode Binary Format Spec

## Overview

This document describes the first-pass `.wrenc` artifact format for Wren
bytecode serialization.

The v1 goal is narrow:
- compile a single source file
- keep the format version-locked
- avoid external module loading
- keep the artifact close to the compiler's existing compiled-module shape

## V1 Requirements

- A magic/header so the loader can reject unrelated files.
- A Wren version stamp so the loader can reject incompatible artifacts.
- Enough module metadata to reload the compiled unit into the VM.
- The compiled function tree.
- Optional debug information.

## Payload Shape

The payload, in order after the 8-byte header (magic `WREN`, one byte each of
major/minor/patch version, one flags byte), is:

1. **Method-name symbol table.** Method calls, super calls, and method
   definitions encode method names as symbol indices into the compiling VM's
   global `vm->methodNames` table. The loading VM's table may assign
   different indices, so the artifact carries the full serializer VM table
   (count-prefixed, length-prefixed strings) and the loader relocates
   `CALL_*`/`SUPER_*`/`METHOD_*` operands at load time. Added by ticket 006;
   see `plan/tickets/006-method-symbol-relocation.md`.
2. **User-declared variable names.** The names of the module's own
   **user-declared** top-level variables (i.e. the slice of
   `ObjModule.variableNames` that was added during compilation of this
   source file, not the slice inherited from the core module beforehand).
3. **The root `ObjFn` tree.** Each function records `arity` (uint8),
   `numUpvalues` (uint16), and `maxSlots` (uint32) first, then its code,
   constants, and optional debug info. Nested functions appear as
   `CONSTANT_FN` entries in constant tables.

Explicitly excluded from the payload:

- **The module name.** This is supplied by the caller at load time, the same
  way `wrenInterpret(vm, module, source)` takes a module name argument
  rather than reading one from the source text.
- **Core-module variable names and values** (`System`, `Object`, `Class`,
  `Fn`, `Fiber`, `List`, `Map`, `Num`, `String`, etc). These are never read
  from the file. The loader must reconstruct them by copying the loading
  VM's own live core module variables into the freshly created module,
  before wiring up the deserialized `ObjFn` tree — the same step
  `compileInModule` performs for a normal compile. See
  `bytecode-objfn-tree.md` for why this matters: `LOAD_MODULE_VAR` and
  `STORE_MODULE_VAR` use plain positional slot indices with no tag
  distinguishing "core" from "user" variables, so the loader must reproduce
  the same slot layout the original compiling VM had, which only works if
  core variables occupy the same leading slots by construction rather than
  by data stored in the file.

The user-declared variable names are only needed so the loader can reserve
the correct number of slots (so `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` indices
line up) and so debug/disassembly output can show real names. Their values
are never stored — they get populated at runtime by the module's own
`STORE_MODULE_VAR` bytecode, same as in a normal compile.

## Constant Table Values

The compiled function tree's constant tables can contain: `null`, `true`/
`false`, numbers, strings, and nested `ObjFn`s. Boolean constants can appear
not just from source literals but also from class/method attribute values
(`#group = true` style attributes), which are emitted through the same
`emitConstant` path as any other literal. Any constant-tag encoding needs a
distinct tag for booleans (or for `true`/`false` individually) in addition to
null, number, string, and function tags.

## V1 Non-Goals

- No stable cross-version bytecode ABI.
- No user-defined import graph.
- No multi-module bundle format.
- No VM snapshot / freeze-image format.
- No obfuscation or encryption layer.

## Design Notes

The first implementation should prefer a conservative, self-describing format
that is easy to load back into the same Wren version it came from.

The exact layout can stay simple and mostly mirror the compiler's existing
internal compiled-module structures. If a field is only useful for a broader
future design, it should be left out of v1.
