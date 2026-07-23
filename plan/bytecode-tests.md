# Testing Plan for Bytecode Serialization

## Goal

Define the tests needed to keep the first-pass bytecode feature stable.

## Existing Test Infrastructure

Wren already has C-based test layers that can be reused for this work.

### Layer 1 - `test/unit/`

Use this for low-level serializer/deserializer tests that do not need to run
full Wren scripts end to end.

Good fits:
- header validation
- version mismatch rejection
- truncated/corrupt payload rejection
- round-trip checks on compiled module structures

### Layer 2 - `test/api/`

Use this for end-to-end tests that compile a simple script, serialize it, load
it back, and execute it through the VM.

Good fits:
- source execution vs serialized execution
- `System.print()` behavior through the host callback
- basic class/function/closure behavior

## V1 Test Buckets

- format tests
- round-trip tests
- version/compatibility tests
- loader failure tests
- execution equivalence tests

## V1 Test Cases

- minimal script loads and runs
- simple class with a method loads and runs
- closure captures local state after reload
- module-level variable declared by the script round-trips and keeps the
  correct `LOAD_MODULE_VAR`/`STORE_MODULE_VAR` slot after reload
- core variables (e.g. `System`, `Object`, `Fn`) resolve correctly after
  reload into a fresh VM (core module slot order is fixed per VM by
  `wrenInitializeCore`, so this mainly guards against the loader forgetting
  the core-import step rather than any load-order variation)
- a boolean constant (e.g. from a class/method attribute value) round-trips
  correctly, not just `null`/number/string constants
- invalid header is rejected
- wrong version is rejected
- truncated file is rejected
- serialized output produces the same observable result as source execution

## Out of Scope For V1

- external module loading tests
- obfuscation/encryption tests
- multi-module bundle tests

## File Strategy

Start with the smallest number of tests that prove the artifact can round-trip
and execute correctly. Expand only when the minimal flow is stable.
