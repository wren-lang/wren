# Bytecode Tickets

## Next

- All planned v1 bytecode tickets are complete. Nice-to-haves are in the backlog.

## Backlog

- [ ] External module loading.
- [ ] Obfuscation or encryption.
- [ ] Multi-module artifact bundles.
- [ ] Stable cross-version bytecode ABI.

## Done

- [x] Wrote `plan/README.md`.
- [x] Wrote `plan/bytecode-scope.md`.
- [x] See `tickets/001-scope-lock.md`.
- [x] See `tickets/002-bytecode-format.md`.
- [x] See `tickets/003-serializer.md` (`src/vm/wren_serialize.c` implements
      `wrenSerializeModule`).
- [x] See `tickets/004-loader.md` (`wrenInterpretBytecode` is implemented and
      verified by the current suite plus ASan).
- [x] See `tickets/005-tests.md` (bytecode test coverage: equivalence,
      rejection, API, format; full suite passes plain and under ASan).
- [x] See `tickets/006-method-symbol-relocation.md` (method-symbol table
      serialized and relocated; host `wrenCall` works for loaded methods).
