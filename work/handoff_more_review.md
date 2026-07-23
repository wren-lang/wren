# Handoff: Review Ticket 004 (Loader) Before Implementation

## Context

This is a small, version-locked fork of Wren (https://wren.io) adding
single-file bytecode serialization: compile a `.wren` source file once,
write it to a binary artifact, and later load+run that artifact without the
original source. Full scope/boundaries are in `plan/bytecode-scope.md` and
`plan/tickets/001-scope-lock.md` — the short version: single file only, no
import graph, no stable cross-version ABI, version-locked to the exact Wren
build that produced it.

Planning lives in `plan/`, tickets in `plan/tickets/001` through `005`.
Ticket order: 001 scope lock (done) -> 002 format (done) -> 003 serializer
(done, implemented) -> **004 loader (planned, not yet implemented — this is
what needs review)** -> 005 tests (not started).

## What's already built (ground truth, not just planned)

`src/vm/wren_serialize.c` is real, committed, working code implementing
`wrenSerializeModule` (ticket 003). It compiles a source file using a
throwaway internal `WrenVM` and writes out a byte artifact: magic `WREN`,
3 version bytes, 1 flags byte, then module metadata (user-declared top-level
variable names only — core variables are deliberately excluded and must be
reconstructed live by the loader), then a recursively-serialized `ObjFn`
tree (bytecode, constant table, arity/numUpvalues/maxSlots, optional debug
info).

**Read this file first.** Ticket 004 is written against its exact byte
layout (tag values, field order, endianness), not against the higher-level
`plan/bytecode-format.md` summary doc, which is looser and was written
before the serializer existed. Where the two disagree, `wren_serialize.c`
is correct and the ticket says so explicitly.

## What needs review

`plan/tickets/004-loader.md` — the loader ticket. This is a planning
document, not code. Nothing has been implemented yet. The ask is: **read it
critically against the actual Wren VM source in `src/vm/`, and find
anything wrong, missing, or underspecified before an implementer starts
writing C.**

## Why extra scrutiny is warranted (not just process box-checking)

This ticket went through several review passes already (mine, as the
current assistant), and each pass found a genuine, previously-undetected
correctness bug in the prior draft:

1. **First draft** copied the serializer's "spin up a throwaway VM, do the
   work, tear it down" pattern for the loader. Wrong — the loader needs to
   load into the *host's real, long-lived VM* (symmetric with
   `wrenInterpret`, not with `wrenSerializeModule`). This also caused a
   secondary bug: the throwaway module was given a `NULL` name, but
   `bindForeignClass`/`bindMethod` (`wren_vm.c:572`, `:359`) unconditionally
   dereference `module->name->value` whenever any `foreign class`/`foreign
   method` declaration's *defining* bytecode runs — not just when called —
   so a `NULL`-named loaded module would crash, not error cleanly, on any
   foreign declaration.
2. **After fixing VM ownership**: a fresh bug appeared in the very fix —
   the corrected breakdown step for creating the module used
   `wrenNewModule(vm, AS_STRING(wrenNewString(vm, module)))` in one
   expression. `wrenNewModule`'s own allocation can trigger a GC, and the
   intermediate name string isn't rooted or reachable from anywhere at that
   point. Had to be split into rooting the name string first, matching the
   existing pattern in `defineClass` (`wren_core.c:1224-1225`) and
   `wrenHasModule` (`wren_vm.c:1970-1971`).
3. **Also found on that same pass**: `wrenDefineVariable` can return `-1`
   (duplicate name) as well as `-2` (too many variables). The ticket's
   variable-slot-reservation step only checked `-2`. A corrupt/hostile
   artifact listing the same own-variable name twice would have been
   silently accepted.
4. **Also found**: the error-reporting plan initially reused
   `WREN_ERROR_COMPILE`/`WREN_RESULT_COMPILE_ERROR` with a `-1` sentinel
   line for artifact-structure failures (bad magic, wrong version,
   truncation, etc). That overloads a field `wren.h` documents as meaning
   something real ("line where the error occurs") with a fake value.
   Resolved by adding a new `WREN_ERROR_LOAD`/`WREN_RESULT_LOAD_ERROR` pair
   instead — additive, and in-scope per ticket 001 (this isn't a stable
   public ABI).

The pattern across all four: each bug looked *plausible* in prose and only
becomes obviously wrong once checked against the actual VM internals
(GC rooting discipline, exact function return values, exact struct/pointer
dereferences). That's a strong signal there may be more of the same kind
still latent in the current draft — this class of bug doesn't reliably get
caught by re-reading your own prose, only by adversarially checking it
against source.

## Specific things to check

- **GC-rooting correctness**, everywhere the ticket describes allocating an
  `Obj*` (`ObjModule`, `ObjFn`, `ObjString`, `ObjClosure`, `ObjFiber`).
  `wrenPushRoot`/`wrenPopRoot` use a fixed 8-slot stack
  (`WREN_MAX_TEMP_ROOTS = 8`, `src/vm/wren_vm.h:11`) shared by the whole VM.
  The ticket's recursive `ObjFn`-tree-rebuilding step (Breakdown step 4) is
  the highest-risk area — verify the described "root per recursion level,
  pop as soon as attached to parent" discipline is actually sufficient and
  doesn't secretly need more roots than assumed at some nesting point.
- **Every claimed function behavior against its actual implementation**,
  not just its name. E.g. the ticket makes claims about
  `wrenDefineVariable`'s return values, `wrenNewFiber`'s stack sizing,
  `bindForeignClass`/`bindMethod`'s dereferencing behavior,
  `compileInModule`'s registration timing. Each of these should be
  re-verified against `src/vm/wren_vm.c`/`wren_value.c`/`wren_compiler.c`
  directly — don't trust the ticket's line-number citations without
  spot-checking a few.
- **The "reject if module name already loaded" decision** (Decisions
  Made). Is checking `wrenHasModule`/`getModule` up front actually
  sufficient, or is there a TOCTOU-style gap (e.g. could `wrenNewString`
  for the name, or some other allocation before registration, trigger a GC
  or a reentrant call that changes `vm->modules` between the check and the
  registration)?
- **The partial-failure/cleanup story.** The ticket accepts that a module
  which fails to load partway through stays registered in `vm->modules` in
  a partially-populated state (mirroring a pre-existing `compileInModule`
  behavior/TODO). Confirm this is actually safe for the loader's case and
  not just copied over without checking whether the loader's partial state
  is safe in the same way `compileInModule`'s is.
- **The new `WREN_ERROR_LOAD`/`WREN_RESULT_LOAD_ERROR` additions to
  `wren.h`.** Check for exhaustiveness: every place the ticket says the
  loader should "reject" or "return a clean error," confirm it's actually
  routed through this new error path consistently, and that nothing falls
  through to an ambiguous or unhandled case.
- **Limits/validation completeness.** The ticket validates `numUpvalues`
  against `MAX_UPVALUES` and `arity` against `MAX_PARAMETERS`. Are there
  other fields read from the artifact that could similarly corrupt VM state
  if out of range (e.g. `maxSlots` itself, which sizes the fiber's stack
  allocation in `wrenNewFiber`) that aren't currently being validated?
- **Whether the "loader lives in `src/vm/wren_serialize.c`, needs internal
  VM access" framing is still right**, or whether a cleaner file split
  makes more sense now that the loader's shape (taking an existing `VM*`,
  no VM lifetime of its own) is simpler than originally planned.
- **The one deliberately open item**: the exact public entry-point
  signature (currently just a leading guess: `WrenInterpretResult
  wrenLoadModule(WrenVM* vm, const char* module, const uint8_t* bytes,
  size_t length)`). Feel free to have opinions here — this was left open on
  purpose as an ergonomics choice, not a behavioral one, but a second
  opinion is welcome.

## How to work

- `plan/tickets/004-loader.md` is the primary document to review.
- Read it alongside `src/vm/wren_serialize.c` (the actual serializer,
  ground truth for the byte format) and `src/vm/wren_vm.c` /
  `wren_value.c` / `wren_compiler.c` (VM internals the loader ticket makes
  claims about).
- `plan/bytecode-deserialize.md`, `plan/bytecode-objfn-tree.md`, and
  `plan/bytecode-format.md` are earlier, higher-level planning docs. They
  are useful background but are not authoritative where they conflict with
  the ticket or with `wren_serialize.c` itself — the ticket supersedes them
  for anything it explicitly resolves.
- Git history for this branch (`bytecode`) shows the iteration described
  above: commits `04ccf4ff` (003 implementation), `da2b1375` (VM-ownership
  correction), `8dda2f1d` (GC bug + error-type fix) are the relevant recent
  ones if you want to see the actual diffs rather than just this summary.
- Nothing has been implemented for ticket 004 yet — there is no code to
  review, only the plan. The output of this review should be either
  corrections to `plan/tickets/004-loader.md` directly, or a list of
  concerns to resolve before implementation starts.
