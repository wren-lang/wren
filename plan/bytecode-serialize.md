# Serialization Plan

This document describes the compiler-side export path for the first-pass bytecode feature.

## Goal

Take the compiled output for a single Wren source file and write it to a
`.wrenc` artifact.

## V1 Scope

- Use the normal Wren compiler and VM to produce the compiled module.
- Serialize the resulting compiled module with minimal transformation.
- Keep the export path local to the compiler/runtime boundary.
- Support optional debug info only if it is easy to carry through cleanly.

## Important Constraint

The compiler is not being turned into a standalone offline compiler in v1.
The safest path is to spin up a normal `WrenVM`, compile normally, then export
the compiled module from that live VM state.

## Export Responsibilities

- write a recognizable file header
- write version metadata
- write the names of the module's own user-declared top-level variables
  (not the core-module variables the module inherited before compilation
  started — see `bytecode-format.md`)
- write the compiled module payload (the root `ObjFn` tree)
- reject unsupported compiler output early

## What This Doc Is Not

- It is not a general-purpose compiler rewrite.
- It is not a portable bytecode ABI design.
- It is not a package/module bundler.

## Implementation Shape

Prefer to reuse the compiler's existing data structures rather than inventing a
new intermediate representation. The export path should be a thin layer over the
compiled objects the VM already has in memory.

The module the compiler used already has core-module variables copied into
its leading slots before compilation begins (mirroring `compileInModule`).
The exporter must record where that boundary was — i.e. only serialize
`ObjModule.variableNames` entries from that boundary onward — so it does not
mistakenly write out core variable names as if they belonged to this module.
