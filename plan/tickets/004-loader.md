# Ticket 004 - Loader

## Goal

Add the VM-side load path that reconstructs a compiled module from a serialized
artifact and executes it.

## Notes

The loader should be conservative. It should validate the header and version,
rebuild the compiled module, and reject malformed or incompatible input early.
It should not attempt to recover from arbitrary broken artifacts.

`foreign class`/`foreign method` declarations compile to ordinary opcodes and
carry no C bindings in the artifact — the actual allocate/finalize/foreign
function pointers are resolved at execution time via the loading VM's
`bindForeignClassFn`/`bindForeignMethodFn` config callbacks, exactly as they
would be for a normal source compile. This is a pre-existing host-
configuration requirement, not something the loader needs to validate or
embed. If the loading host doesn't register matching bindings, a foreign
constructor/method call fails at that point with a normal runtime error, the
same as running the original source on a VM missing those bindings would.

## Acceptance Criteria

- A loader entry point exists.
- It rejects malformed or incompatible artifacts cleanly.
- It can hand a loaded module to the VM for execution.
- It reconstructs enough runtime state for the VM to run the module normally.
- It does not require the original source file.
