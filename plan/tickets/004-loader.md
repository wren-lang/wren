# Ticket 004 - Loader

## Goal

Add the VM-side load path that reconstructs a compiled module from a serialized
artifact and executes it.

## Notes

The loader should be conservative. It should validate the header and version,
rebuild the compiled module, and reject malformed or incompatible input early.
It should not attempt to recover from arbitrary broken artifacts.

## Acceptance Criteria

- A loader entry point exists.
- It rejects malformed or incompatible artifacts cleanly.
- It can hand a loaded module to the VM for execution.
- It reconstructs enough runtime state for the VM to run the module normally.
- It does not require the original source file.
