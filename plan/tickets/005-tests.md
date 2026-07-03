# Ticket 005 - Tests

## Status

Shaped for implementation.

## Goal

Build enough bytecode test coverage to make the first release trustworthy:
source execution and bytecode execution should match for representative Wren
programs, malformed artifacts should fail cleanly before execution, and the
public serializer/loader API should behave predictably for hosts.

## Notes

Ticket 004 already landed the loader and a useful first API test. Current
coverage includes a basic round trip, duplicate module-name rejection, retry
after failed structural load, bad magic/version/flags, trailing-byte rejection,
runtime-error separation, debug-stripped execution, and deeply nested function
constants.

Ticket 005 should not try to turn the v1 loader into a verifier. The loader's
trust boundary remains: artifacts are expected to come from the matching
serializer/compiler, while the loader validates the container, metadata, counts,
lengths, and safe VM object reconstruction. Tests should focus on that boundary
and on user-visible behavior.

The release gate should include both targeted C/API tests and end-to-end
source-vs-bytecode equivalence cases. Prefer small, readable fixtures over a
large golden corpus. Golden bytes are brittle because the format is explicitly
version-locked and internal; behavior and rejection semantics matter more for
this first release than byte-for-byte stability across commits.

## Implementation Todos

### Execution Equivalence

Add representative programs that run once from source and once from serialized
bytecode, then compare observable output and result code.

- [ ] Add equivalence coverage for top-level statements and module variables.
- [ ] Add equivalence coverage for classes, methods, static methods,
  constructors, and fields.
- [ ] Add equivalence coverage for inheritance and `super`.
- [ ] Add equivalence coverage for closures that capture locals and survive
  nested `ObjFn` reload.
- [ ] Add equivalence coverage for loops, branches, short-circuit boolean logic,
  and returns.
- [ ] Add equivalence coverage for list/map/string/number constants and
  operations.
- [ ] Add equivalence coverage for boolean and null constants in constant
  tables.
- [ ] Add equivalence coverage for method attributes or another known source
  shape that emits boolean constants.
- [ ] Add equivalence coverage for runtime errors, including stack traces when
  debug info is present.
- [ ] Add equivalence coverage for debug-stripped artifacts, ensuring runtime
  errors are memory-safe and produce a clean runtime-error result.

### Loader Rejection And Corruption Handling

Extend malformed-artifact tests beyond the current header/trailing-byte cases.
Use valid serializer output, mutate one field at a time, and assert
`WREN_RESULT_LOAD_ERROR` plus one `WREN_ERROR_LOAD` callback where applicable.

- [ ] Add malformed-artifact coverage for every possible truncation point in a
  bounded sweep over one small serialized artifact.
- [ ] Add malformed-artifact coverage for unknown header flags.
- [ ] Add malformed-artifact coverage for mismatched debug line count.
- [ ] Add malformed-artifact coverage for invalid constant tag.
- [ ] Add malformed-artifact coverage for invalid root `numUpvalues`.
- [ ] Add malformed-artifact coverage for invalid function metadata: arity too
  large, zero `maxSlots`, and `maxSlots < arity + 1`.
- [ ] Add malformed-artifact coverage for impossible or excessive
  counts/lengths that should be rejected before allocation.
- [ ] Add malformed-artifact coverage for empty and overlong serialized
  module-variable names.
- [ ] Add malformed-artifact coverage for duplicate serialized module-variable
  names, including collision with copied core names such as `System`.
- [ ] Add malformed-artifact coverage for trailing bytes after an otherwise
  valid function tree.

### API And Host Behavior

Make sure bytecode-loaded modules behave like source-compiled modules from a
host's point of view.

- [ ] Add API coverage proving successful load registers the requested module
  name in `vm->modules`.
- [ ] Add API coverage proving `wrenHasModule`, `wrenHasVariable`,
  `wrenGetVariable`, and `wrenCall` work against a bytecode-loaded module the
  same way they work after `wrenInterpret`.
- [ ] Add API coverage proving structural load failure does not register or
  poison the requested module name.
- [ ] Add API coverage proving duplicate requested module name is rejected
  cleanly.
- [ ] Add API coverage proving the loader uses the caller's configured
  `writeFn` and `errorFn`.
- [ ] Add API coverage proving `WREN_ERROR_LOAD` is used only for pre-execution
  structural failures.
- [ ] Add API coverage proving runtime failures from loaded code return
  `WREN_RESULT_RUNTIME_ERROR`, not `WREN_RESULT_LOAD_ERROR`.
- [ ] Add API coverage proving serializer failure from bad source remains a
  compile error, not a load error.

### Format And Serializer Sanity

Add focused tests that catch accidental serializer/loader drift without making a
long-term ABI promise.

- [ ] Add format sanity coverage for `WREN` magic, exact version bytes, and
  expected flags.
- [ ] Add format sanity coverage proving the debug-info flag is present only
  when debug serialization is requested.
- [ ] Add format sanity coverage proving serialized artifacts without debug info
  are accepted and get synthesized safe debug state at load time.
- [ ] Add format sanity coverage proving serializer output can be loaded by a
  fresh VM with a different configured callback context.
- [ ] Add format sanity coverage proving the serializer does not include the
  source module name; the loader-supplied module name is the one registered.

### Memory Safety And Release Gate

Run the test suite under the same modes used to verify ticket 004.

- [ ] Verify full `python3 util/test.py` passes.
- [ ] Verify bytecode-focused subset passes, e.g.
  `python3 util/test.py api/bytecode_loader`.
- [ ] Verify debug build with AddressSanitizer passes the full suite with leak
  detection disabled if that remains the local convention.
- [ ] Verify malformed-artifact tests do not crash or hang under ASan.
- [ ] Verify repeated serialize/load/free cycles do not grow unbounded memory in
  normal test runs.

### Implementation Order

- [ ] Add a reusable C test helper that serializes source, loads bytecode into a
  fresh VM, captures output/errors, and compares with source execution.
- [ ] Expand `test/api/bytecode_loader.c` with API and malformed-artifact
  coverage.
- [ ] Add a small set of equivalence fixtures or inline source strings for the
  representative language cases above.
- [ ] Add bounded truncation/mutation tests for the serialized artifact fields
  that ticket 004 promises to validate.
- [ ] Run the normal suite and ASan suite as the release gate.

## Nice-To-Have, Not Required For First Release

- A small mutation/fuzz harness for serialized artifacts.
- Randomized source generation.
- Tests for executing imported modules from bytecode, because bundled external
  module loading is out of v1 scope.
- Cross-version artifact compatibility tests, because v1 is version-locked.
- Obfuscation/encryption tests.

## Acceptance Criteria

- Source-vs-bytecode equivalence tests cover top-level variables, classes,
  methods, inheritance, closures, control flow, core collection/string/number
  behavior, and representative constants.
- Loader rejection tests cover bad magic, wrong version, unknown flags,
  truncation, trailing bytes, invalid constant tags, invalid function metadata,
  bad debug line counts, bad module-variable names, duplicate names, and
  oversized counts/lengths.
- API tests prove successful bytecode loads register the requested module name
  and expose variables/callable methods through the normal public API.
- API tests prove failed structural loads do not register the requested module
  name and can be retried in the same VM.
- Error-path tests distinguish `WREN_RESULT_LOAD_ERROR`/`WREN_ERROR_LOAD` from
  normal compile and runtime errors.
- Debug-info tests cover both serialized debug info and debug-stripped
  artifacts.
- Tests remain within the v1 single-file scope and do not depend on bundled
  external module loading.
- The full suite passes with `python3 util/test.py`.
- The bytecode API subset passes with `python3 util/test.py api/bytecode_loader`.
- The full suite passes under the ticket-004 ASan configuration, with malformed
  artifacts exercising rejection paths without crashes.
