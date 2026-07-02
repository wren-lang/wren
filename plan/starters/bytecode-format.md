# Bytecode Binary Format Spec

## Overview

A `.wrenc` file is a serialized `ObjFn` tree — the root function of a compiled
module — plus the metadata needed to load it cleanly into any Wren VM of the
same version.

## File Layout

```
[Header]
[Symbol Name Table]
[Module Variable Name Table]
[Root ObjFn]  ← recursive, contains nested ObjFns in its constant table
```

---

## Header (8 bytes)

| Field     | Type    | Value                          |
|-----------|---------|--------------------------------|
| `magic`   | u8 × 4  | `'W' 'R' 'E' 'N'`             |
| `major`   | u8      | `WREN_VERSION_MAJOR` (0)       |
| `minor`   | u8      | `WREN_VERSION_MINOR` (4)       |
| `patch`   | u8      | `WREN_VERSION_PATCH` (0)       |
| `flags`   | u8      | bit 0 = `HAS_DEBUG_INFO`       |

The loader rejects any file where `major`, `minor`, or `patch` don't match the
running VM. No cross-version loading.

---

## Symbol Name Table

Collects every method name referenced by a `CALL`, `SUPER`, `METHOD_INSTANCE`,
or `METHOD_STATIC` instruction in the entire `ObjFn` tree. The bytecode stores
**local indices** (into this table) rather than VM-global symbol IDs.

At load time, each name is resolved via `wrenSymbolTableEnsure` against the
loading VM's `methodNames` table, producing a `remap[]` array used during the
bytecode patch pass.

```
count    : u16
entries  : count × { length: u16, bytes: u8[length] }
```

### Example

Source: `System.print("hello")`

**Compiler VM** (the machine that ran `wrenc`) has built up `methodNames` like:

```
vm->methodNames[0]  = "init new()"
vm->methodNames[1]  = "+"
...
vm->methodNames[42] = "print(_)"   ← happened to land here
```

The compiler emits `CALL_1 42` into the bytecode stream. During serialization
we collect all referenced symbol IDs, look up their names, and write the name
table:

```
count = 1
[0] = "print(_)"
```

Then we rewrite the bytecode arg from the VM-global `42` down to the local
index `0` — the position of `"print(_)"` in *our* name table:

```
before:  CALL_1  42   ← VM-global, meaningless elsewhere
after:   CALL_1   0   ← local index into symbol name table
```

**Loading VM** (e.g. a microcontroller) reads the name table and resolves each
entry against its own `methodNames` table using `wrenSymbolTableEnsure`, which
adds the name if it isn't already there and returns its index:

```c
remap[0] = wrenSymbolTableEnsure(vm, &vm->methodNames, "print(_)", 8);
// → 31  (different VM, different order of module loading)
```

Now `remap` is `[0 → 31]`. A single linear pass over the bytecode finds every
`CALL_1` (and all other symbol-carrying instructions), reads the 2-byte arg,
treats it as a local index, and writes back the remapped VM-global value:

```c
// pseudocode for the patch walk
uint16_t local_idx = (code[ip+1] << 8) | code[ip+2];
uint16_t vm_idx    = remap[local_idx];
code[ip+1] = (vm_idx >> 8) & 0xff;
code[ip+2] =  vm_idx       & 0xff;
```

After the pass, `CALL_1 0` has become `CALL_1 31` in memory — exactly what the
compiler would have produced had it compiled the source natively on this VM.
The bytecode is now ready to execute.

---

## Module Variable Name Table

The names of all module-level variables defined by this module, in slot order.
Used to reconstruct `module->variableNames` on load. The slot indices in
`LOAD_MODULE_VAR` / `STORE_MODULE_VAR` are stable for whole-module compilation
(always start at 0), so **no remapping is needed** — just the names for the
module's symbol table and debug output.

```
count    : u16
entries  : count × { length: u16, bytes: u8[length] }
```

### Example

Source:

```wren
var pi = 3.14159
var greeting = "hello"
var counter = 0
```

Module variable name table:

```
count = 3
[0] = "pi"
[1] = "greeting"
[2] = "counter"
```

The bytecode for `counter = counter + 1` would contain:

```
LOAD_MODULE_VAR   2   ← slot 2, "counter"
CONSTANT          0   ← 1
CALL_1           ...  ← "+"
STORE_MODULE_VAR  2   ← slot 2, "counter"
```

Slot `2` means "counter" in both the compiler VM and the loading VM because
the slots are assigned sequentially as the module is compiled top-to-bottom —
they're positional, not registry-based. The name table just lets the loading
VM reconstruct `module->variableNames` so that the debugger and disassembler
can print `'counter'` instead of `'2'`.

---

## ObjFn (recursive)

Each function in the tree is serialized the same way. Nested functions appear
as `TAG_FN` entries in the parent's constant table.

```
maxSlots     : u8
numUpvalues  : u8
arity        : u8

--- constants ---
const_count  : u16
const[]      : const_count × tagged Value (see below)

--- bytecode ---
code_len     : u16
code[]       : u8[code_len]   ← CALL/SUPER/METHOD args are local symbol indices

--- debug (only if HAS_DEBUG_INFO flag is set) ---
name_len     : u16
name         : u8[name_len]
line_count   : u16            ← must equal code_len
lines[]      : u16[line_count]
```

### Constant Tags

| Tag  | u8 | Payload                                    |
|------|----|--------------------------------------------|
| NULL |  0 | (none)                                     |
| NUM  |  1 | f64 (8 bytes, little-endian)               |
| STR  |  2 | `{ length: u16, bytes: u8[length] }`       |
| FN   |  3 | ObjFn (recurse)                            |

These are the only types that can appear in a compile-time constant table.
Runtime values (instances, maps, lists, fibers) are never constants.

### Upvalue capture info

`CLOSURE` instructions are followed inline in the bytecode by `numUpvalues × 2`
bytes — one `isLocal` byte and one `index` byte per upvalue. This is already
part of `code[]` and requires no special handling during serialization.

---

## Load-time Steps

1. Read and validate header (magic + version).
2. Read symbol name table → resolve each name in loading VM → build `remap[]`.
3. Read module variable name table → reconstruct `module->variableNames`.
4. Deserialize root `ObjFn` recursively (constants first, then bytecode).
5. Walk bytecode, patch all `CALL`/`SUPER`/`METHOD_INSTANCE`/`METHOD_STATIC`
   symbol args through `remap[]`.
6. Wire `fn->module` pointer on every deserialized `ObjFn`.
7. Hand root `ObjFn` to the VM as if `wrenCompile` had just returned it.

---

## Instructions That Carry Symbol IDs (must be patched)

| Instruction(s)             | Arg layout                                    |
|----------------------------|-----------------------------------------------|
| `CALL_0` … `CALL_16`       | `[opcode][sym hi][sym lo]`                    |
| `SUPER_0` … `SUPER_16`     | `[opcode][sym hi][sym lo][super hi][super lo]`|
| `METHOD_INSTANCE`          | `[opcode][sym hi][sym lo]`                    |
| `METHOD_STATIC`            | `[opcode][sym hi][sym lo]`                    |

All other instructions step over cleanly using `getByteCountForArguments`.
