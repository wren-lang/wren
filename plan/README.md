# Bytecode Planning

This directory contains the working notes and design docs for Wren bytecode serialization.

Current direction:
- Start with a minimal, version-locked fork.
- Compile a single Wren source file first.
- Keep external module loading out of v1.
- Focus on core runtime support and a clean export/import path.

## Docs

- `bytecode-scope.md` - v1 scope, non-goals, and explicit boundaries.
- `bytecode-format.md` - serialized artifact layout and versioning rules.
- `bytecode-serialize.md` - compiler/export path.
- `bytecode-deserialize.md` - VM/load path.
- `bytecode-tests.md` - test strategy and acceptance matrix.
- `bytecode-wrenc-vm.md` - tool shape and compile/run workflow.
- `bytecode-obfuscation.md` - source packing / obfuscation notes, likely out of scope for v1.
- `bytecode-objfn-tree.md` - deeper internal notes on object/function structure.

## Working Notes

The docs here are intentionally incremental. `bytecode-scope.md` should be treated as the primary reference for what is in and out of scope for the first pass.
