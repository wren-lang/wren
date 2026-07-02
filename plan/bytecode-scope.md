# Bytecode Serialization — Scope & Implementation Plan

Reference: [Issue #535](https://github.com/wren-lang/wren/issues/535)

## Scope Estimate

| Piece | Lines of C | Risk |
|---|---|---|
| Binary format + header | ~50 | Low |
| `wrenSerializeFn` | ~150 | Low |
| `wrenDeserializeFn` | ~200 | Low |
| Method symbol remap pass | ~100 | Medium |
| `wrenc` CLI wrapper | ~100 | Low |
| Tests | ~200 | Low |
| **Total** | **~800** | |

All additive — no existing VM code needs to change.

## Files to Create

- `src/vm/wren_serialize.h` — public API
- `src/vm/wren_serialize.c` — serialize + deserialize + symbol remap
- `util/wrenc.c` — standalone compiler CLI
- `test/serialize/` — test scripts

## Key Constraints

- `wrenc` must spin up a lightweight `WrenVM` to compile (compiler is too tightly coupled to the VM to run standalone)
- Bytecode serialization is **whole-module only** — no REPL snippets
- The binary format must carry a version header so the VM can reject stale bytecode
