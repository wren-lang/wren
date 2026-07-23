# Known Bugs

## Pre-existing upstream: heap-use-after-free in compiler under `WREN_DEBUG_GC_STRESS`

**Status:** FIXED — landed in commit `d7429346` ("Apply upstream compiler
GC-safety fix", `src/vm/wren_compiler.c`). Pre-existing upstream Wren bug —
*not* introduced by the bytecode serializer/loader work. Reproduced at commit
`ab2d4606`. This entry is kept as the investigation record.

**Confirmed root cause:** `wrenCompile()` primes the first two tokens by
calling `nextToken()` **before** `initCompiler()` runs. `initCompiler()`
(`wren_compiler.c:538`) is what sets `vm->compiler`, and
`wrenMarkCompiler()` — reached only through `vm->compiler`
(`wren_vm.c:166`) — is the *only* thing that marks the parser's token values
(`parser->current.value` / `previous.value` / `next.value`,
`wren_compiler.c:3903-3905`). So during those two priming scans the token
`ObjString` values are invisible to the GC: a collection triggered mid-scan
(e.g. by `readName`'s buffer writes) sweeps a token string that
`parser.next.value`/`current.value` still reference. The next GC after
`initCompiler` registers the compiler then grays the dangling `Value` — the
ASan-visible use-after-free. The window exists on *every* `wrenCompile` call
(also at runtime for imports), not just VM init; stress mode just makes it
deterministic.

**The fix (landed):** in `wrenCompile()`: (1) zero-init `parser.current` and
`parser.previous` (not just `parser.next`) so `wrenMarkCompiler` never reads
uninitialized token values, then (2) move `initCompiler()` *above* the two
priming `nextToken()` calls so `vm->compiler` is registered before any token
is scanned. ~10 lines, no behavior change otherwise (`initCompiler` never
reads tokens). Verified: with the fix, ASan + `WREN_DEBUG_GC_STRESS 1` gets
through `wrenNewVM()` with zero reports, and the full normal-mode suite
(867 tests) passes.

**Residual (separate) stress-mode bugs remain:** even with this fix, some
tests fail with *compile errors* (exit 66) under `WREN_DEBUG_GC_STRESS 1`
(e.g. `test/language/class/methods.wren`, `test/core/string/plus.wren`) —
verified identical at unmodified HEAD, so pre-existing and unrelated to this
fix. No ASan reports accompany them, suggesting a semantic stress bug (some
other compiler-held value being legitimately collected) rather than another
UAF. Needs its own investigation before GC-stress mode is fully trustworthy.

**Impact:**

- `WREN_DEBUG_GC_STRESS 1` + AddressSanitizer crashes during `wrenNewVM()`
  (while compiling the core module), before any user code runs. This makes
  GC-stress mode **unusable for validating new GC-sensitive code** (it blocked
  ASan verification of the ticket-004 loader's rooting discipline).
- Without ASan, GC-stress runs "pass" silently — the freed memory is read
  while its old contents are still intact, so nothing observable happens.
- The same window exists under normal (non-stress) GC at very low
  probability: a token's `ObjString` value can be collected between its
  creation in the scanner and being marked, if a GC lands on exactly the
  wrong intervening allocation.

**Reproduction:**

1. In `src/vm/wren_common.h`, set `#define WREN_DEBUG_GC_STRESS 1`.
2. Build the test binary with ASan:

   ```
   clang -fsanitize=address -g -O0 -DWREN_OPT_META=1 -DWREN_OPT_RANDOM=1 \
     -Isrc/include -Isrc/vm -Isrc/optional -Itest -Itest/api \
     src/vm/*.c src/optional/*.c test/main.c test/test.c test/api/*.c \
     -o wren_test_asan -lm
   ```

3. Run it against any test file. ASan reports heap-use-after-free
   immediately, inside `wrenNewVM` → `wrenInitializeCore`.

**ASan trace summary (observed 2026-07-03, macOS arm64, commit ab2d4606):**

- **Read:** `wrenGrayObj` (`wren_value.c:981`) ← `wrenGrayValue`
  (`wren_value.c:1002`) ← `wrenMarkCompiler` (`wren_compiler.c:3903`) ←
  `wrenCollectGarbage` (`wren_vm.c:166`), during a GC triggered from
  `initCompiler`'s `wrenNewMap` (`wren_compiler.c:576`).
- **Freed by:** an earlier stress-mode GC triggered from `wrenByteBufferWrite`
  inside the scanner's `readName` (`wren_compiler.c:813`), which swept the
  string via `wrenFreeObj` (`wren_value.c:1287`).
- **Allocated by:** `wrenNewStringLength` inside `readName`
  (`wren_compiler.c:833`) — i.e. a scanner token's string value.

**Interpretation:** confirmed — see "Confirmed root cause" above. The freed
GC at `readName:813` fired during `wrenCompile`'s second priming
`nextToken()` call, while `vm->compiler` was still `NULL` (init happens at
`wren_compiler.c:3794`, after the priming at :3787-3789). The graying GC at
`initCompiler`'s `wrenNewMap` then read the dangling
`parser->current.value`. The earlier hypothesis that the fix belonged in
`wrenMarkCompiler` was wrong: the marker already handles all three token
values; the bug is that it isn't reachable yet when the first tokens are
scanned. Fix belongs in `wrenCompile`'s setup ordering.

**Suggested next steps:**

- File/track the residual stress-mode compile errors (see above) separately;
  they also predate the bytecode work.
- Once stress mode is fully clean, re-run the ticket-004 loader under
  `WREN_DEBUG_GC_STRESS 1` + ASan — that combination is the definitive test
  for the loader's temp-rooting discipline.
