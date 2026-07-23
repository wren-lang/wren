# Bytecode Serialization — Issue #535

**Issue:** [#535 — Save (and load) compiled bytecode?](https://github.com/wren-lang/wren/issues/535)
**Status:** Open (filed 2018, still unresolved as of 2023)

---

## The Request

A way to compile Wren source code to bytecode ahead of time — on a desktop — and then load and execute that bytecode on a target device (e.g. microcontroller) without needing the source code or a compiler at runtime.

Think `luac` for Wren: a `wrenc` that produces a portable binary that the VM can load directly.

---

## Why It Matters

Primary use cases raised in the thread:

1. **Embedded / microcontroller use.** Example target: STM32L432 (256K flash, 64K SRAM, 80 MHz ARM Cortex M4F). Currently Wren requires loading the full source into RAM and then compiling it, consuming memory twice. On a 64K device that leaves very little room for the rest of the program.

2. **Avoiding runtime compilation overhead.** Small CPUs pay a real cost compiling scripts at startup on every boot.

3. **Not distributing source code.** Many commercial/client projects cannot ship source. Bytecode (even without strong obfuscation) raises the cost of reverse engineering. This applies to desktop/server use too, not just embedded.

4. **Faster startup / pre-compilation of Wren's own core libs.** Wren's built-in modules could be shipped pre-compiled to bytecode, speeding up VM startup.

---

## Core Challenges (per @munificent)

1. **Bytecode is an unstable implementation detail.** There is no public, versioned bytecode ABI. Stabilizing it (or versioning it with a format header) would be needed before shipping serialized code. Users would inevitably treat it as a long-lived artifact even if told not to.

2. **Compiler and runtime are tightly coupled.** Constants (including function objects) are turned into live heap values at compile time and stored directly in the constant table. The compiler looks up method symbols in the VM's global ID table, imports already-compiled modules, etc. A serialization layer would need to decouple all of this.

3. **REPL / incremental compilation.** The compiler can compile new code into the context of an existing live module — it sees what top-level variables are already in scope. A hard compile-then-serialize model would need to compile entire modules at once and give up per-expression context.

---

## Proposed Approaches

### Option A — Full VM image / "freeze" snapshot
Compile all sources into a running VM, then serialize the entire VM memory image to disk (like Smalltalk images). Load the image on the target — no compilation happens at load time.

- **Pros:** Conceptually simple; no new bytecode format needed; can omit debug info to save space.
- **Cons:** Image is VM-version locked; harder to share individual modules; images can be large.
- @mhermier noted that duplicating all built-in modules in every image is a downside.

### Option B — Per-module compiled bytecode
Compile one module at a time to a binary file. Load it like a shared library.

- Requires a stable (versioned) bytecode format.
- Requires decoupling the compiler from live runtime state (method symbol tables, constant heap allocation, etc.).
- @mhermier outlined three changes needed: de-hard-code core symbol injection, make module binding work like method binding (patch global offsets at load time), serialize *before* patching.
- More composable and familiar to users of other scripting VMs.

### Option C — Obfuscated/packed source
Pack and optionally encrypt source text, unpack into memory at load time, pass to `wrenInterpret`. Not "real" bytecode but addresses the source-distribution concern with minimal VM changes.

- Noted by @ruby0x1 as sufficient for some use cases.
- Does **not** address memory or startup-time concerns on very constrained devices.

---

## Related API Pain Point

`wrenInterpret` currently copies the source string, meaning an 8K source file requires 16K of RAM during compilation. An additional "take ownership" API would let callers avoid the copy on constrained systems.

---

## Current Status

No implementation has landed. The issue remains open. Contributors (@mhermier, @ruby0x1) acknowledge it is desirable but note significant internal refactoring is required. Several community members have shipped Wren on microcontrollers (Arduino M4, ESP32) by trimming the VM, but without bytecode serialization.
