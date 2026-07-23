# Ticket 001 - Scope Lock

## Goal

Confirm the first-pass bytecode feature stays narrow: single source file,
version-locked, and no user-defined imports.

## Notes

The point of this ticket is to stop the feature from drifting into a larger
module system or a long-lived bytecode ABI. The first pass should be a clean,
maintainable fork, not a future-proof platform.

## Acceptance Criteria

- v1 scope is explicitly single-file only.
- External module loading is out of scope.
- Obfuscation is out of scope.
- The artifact is treated as version-locked rather than a stable ABI.
- The core runtime surface is limited to what the minimal loader needs.
- `wren-cli` stays out of the bytecode contract.
