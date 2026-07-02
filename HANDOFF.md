# Bytecode Handoff

## Goal

We are exploring a first-pass Wren bytecode serialization feature in the Wren
source tree.

The current intent is a clean, minimal fork rather than an upstream-ready ABI:
- compile a single source file
- no user-defined imports in v1
- version-locked artifact
- keep the compiler/export path and VM/load path as small as possible

## What We Have Decided

- `plan/README.md` is the index for the bytecode planning docs.
- `plan/bytecode-scope.md` is the v1 boundary document.
- `plan/roadmap.md` and `plan/tickets.md` are lightweight local tracking.
- `plan/tickets/` contains individual markdown tickets for the first steps.
- Debug info stays in v1.
- Obfuscation is out of scope for v1.
- External module loading is out of scope for v1.
- The artifact is intended to be version-locked, not a stable cross-version ABI.

## Confirmed Code Facts

- Wren already uses a host `writeFn` for `System.print()` and related output.
- `wren_core.wren` defines `class System` in core Wren.
- `wrenCompile()` returns a root `ObjFn` representing a compiled module.
- Nested functions are stored in constant tables as nested `ObjFn`s.
- Module-level variables are stored in `ObjModule.variableNames` and
  `ObjModule.variables`.
- Bytecode instructions `LOAD_MODULE_VAR` and `STORE_MODULE_VAR` use slot
  indices into the module variable table.

## Current Ticket State

- `001-scope-lock.md` is effectively done for now.
- `002-bytecode-format.md` is the active design ticket.
- `003-serializer.md`, `004-loader.md`, and `005-tests.md` are the main
  implementation tickets after the format is settled.

## Open Questions For The Next Planning Pass

- What exact module metadata does v1 need?
- Do we need to serialize module name plus module variable names?
- How closely should the format mirror current compiler internals?
- Is there any small amount of normalization worth keeping, or should we stay
  as close as possible to the compiled structures?

## Recommendation For The Next Model Pass

Use a stronger planning model before finalizing the format and loader details.
This is where compiler/runtime coupling matters, especially around module
metadata and what the loader must reconstruct.
