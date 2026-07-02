# Tooling Shape

This document describes the first-pass tool split for bytecode serialization.

## Goal

Provide a desktop-side way to compile a Wren source file into a serialized
artifact, and a runtime-side way to load and execute that artifact.

## Suggested Shape

- `wrenc`: desktop compiler/export tool.
- `wrenvm` or `wren-run`: minimal runtime loader for serialized artifacts.
- `wren-cli`: remains the richer developer shell with extra host modules and REPL conveniences.

## V1 Constraints

- Keep the compiler/export path in the core Wren tree.
- Keep the runtime/load path small and close to the VM.
- Do not require `wren-cli` for the serialized artifact workflow.

## Important Note

The compiler still needs a normal `WrenVM` during the compile step. The first
pass should use that existing path rather than trying to detach the compiler
from the VM.

## Non-Goals

- No full module ecosystem.
- No package manager.
- No external import resolution for v1.
