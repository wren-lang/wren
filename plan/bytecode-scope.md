# Bytecode Scope

This document defines the first-pass scope for Wren bytecode serialization.

## Goal

Allow a single Wren source file to be compiled on a desktop host into a serialized artifact, then loaded and executed by a VM without needing source at runtime.

## V1 Scope

- Compile one source file into a serialized artifact.
- Support normal Wren language features within that file, including multiple classes, functions, closures, loops, and control flow.
- Preserve `System.print()` and other core runtime hooks that are already part of Wren's base environment.
- Keep the artifact tied to a specific Wren version or narrow version family.
- Add a loader path in the VM that can execute the serialized artifact.
- Keep the compiler/export path and VM/import path as small and localized as possible.

## V1 Non-Goals

- No user-defined import graph handling.
- No external module loading from the file system.
- No package manager or dependency resolver.
- No bundled multi-module artifact format.
- No attempt to define a long-lived stable bytecode ABI.
- No full VM snapshot or freeze-image format.
- No obfuscation/encryption layer as part of the first pass.

## Assumptions

- The compiler remains part of the core Wren source tree.
- The runtime continues to use the existing host callbacks for output, errors, and module resolution where applicable.
- `wren-cli` remains a separate convenience layer and is not required for the v1 bytecode path.
- The bytecode format may be version-locked and intentionally narrow.

## Implications

- The serializer can stay close to existing compiled module structures.
- The loader can be simpler because it only needs to hydrate one module.
- Tests can focus on round-trip behavior, version rejection, and corruption handling.
- Future support for external modules can be added later without blocking the initial implementation.
