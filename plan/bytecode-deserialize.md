# Deserialization Plan

This document describes the VM-side load path for the first-pass bytecode feature.

## Goal

Load a serialized `.wrenc` artifact into a Wren VM and execute it without
needing the original source file.

## V1 Scope

- Validate the file header and version.
- Create a fresh module and populate its leading variable slots from the
  loading VM's own live core module, in the same order `compileInModule`
  would — the artifact never carries core variable data itself.
- Reserve slots for the module's user-declared variable names (read from the
  artifact) immediately after the core slots, so `LOAD_MODULE_VAR` and
  `STORE_MODULE_VAR` indices in the deserialized bytecode line up correctly.
- Rebuild the compiled module into live VM objects.
- Wire the module into the runtime so it can execute like a normal compiled script.
- Fail cleanly on malformed, truncated, or incompatible files.

## V1 Constraints

- No external module loader.
- No import graph resolution.
- No cross-version compatibility guarantee.
- No attempt to support artifacts produced by unrelated Wren builds.

## Runtime Expectations

The load path should assume the VM already knows how to execute compiled Wren
objects. The deserializer's job is to reconstruct enough state for the VM to
pick up where the compiler would normally leave off.

## Implementation Shape

Keep the loader small and conservative. Prefer rejecting questionable input over
trying to recover from every possible malformed artifact.
