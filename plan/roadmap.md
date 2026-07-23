# Bytecode Roadmap

This roadmap tracks the first-pass bytecode work as a small, version-locked
fork of Wren.

Status: Phases 0-4 are complete (tickets 001-006). Phase 5 is deliberately
deferred — v1 ships as library API only.

## Phase 0 - Scope Lock

- Confirm v1 is single-file only.
- Confirm external imports are out of scope.
- Confirm the artifact is version-locked.

## Phase 1 - Format

- Define the minimal `.wrenc` layout.
- Decide what metadata must be stored to reload a compiled module.

## Phase 2 - Export Path

- Add a serializer that writes compiled module state to disk or memory.
- Keep the export layer close to the existing compiler/VM path.

## Phase 3 - Load Path

- Add a loader that can reconstruct the compiled module in the VM.
- Make invalid or incompatible artifacts fail cleanly.

## Phase 4 - Tests

- Add format and version rejection tests.
- Add source-vs-bytecode execution tests.

## Phase 5 - Tooling (deferred, not part of v1)

- Decide whether the desktop compiler and runtime loader are separate binaries
  or a subcommand split.
- Keep `wren-cli` as the richer host/tooling layer.
