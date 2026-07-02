# Bytecode Obfuscation

## Honest Assessment

`.wrenc` files are **not secure by default**. The binary format is
well-structured and self-describing. A disassembler is trivial to write
(~300 lines of C) and `wrenDumpCode` already exists as a starting point.

What's readable in a plain `.wrenc` file:

| Data | Location in file |
|---|---|
| All method names your code calls | Symbol name table |
| All module-level variable names | Module variable name table |
| All string literals | `TAG_STR` entries in constant tables |
| Function names + line numbers | Debug section (if `HAS_DEBUG_INFO` set) |
| Control flow structure | Jump/branch instructions |

Bytecode raises the bar from "open in editor" to "run a disassembler" — an
afternoon's work for someone motivated. It is **not** a cryptographic
protection.

---

## What Actually Helps

### 1. Strip debug info (free, always do this for production)

Ship with `HAS_DEBUG_INFO = 0`. Eliminates function names and source line
numbers. Costs you readable stack traces at runtime.

### 2. XOR-obfuscate the string tables

The two name tables are the most human-readable part of the binary. XOR-ing
them against a key eliminates plain-text method and variable names, and defeats
`strings -` inspection.

The same scheme extends naturally to `TAG_STR` constant entries if string
literal exposure is also a concern.

### 3. Encrypt the whole file (strongest option)

AES-encrypt the entire `.wrenc` payload, decrypt into a memory buffer at load
time, never write plaintext to disk. The key lives in the host firmware. Used
by most commercial game engines for asset packs.

### 4. Accept the limits

Anything executing on the target device can be dumped from RAM by a sufficiently
motivated attacker with physical access. A legal contract protects IP better
than obfuscation. The real value of bytecode is **not shipping editable source**,
**faster startup**, and **smaller RAM footprint** — not strong IP protection.

---

## XOR Obfuscation Design

### What gets XOR'd

- Symbol name table entries (method names)
- Module variable name table entries (variable names)
- Optionally: `TAG_STR` constant table entries (string literals)

### Key source

Controlled by a field in `WrenConfiguration`:

```c
typedef struct {
  // ... existing fields ...
  uint32_t bytecodeObfuscationKey;  // 0 = disabled
} WrenConfiguration;
```

The host application supplies the key. On a microcontroller, it's baked into
the firmware at build time — separate from the Wren VM source. An attacker
needs both the `.wrenc` file *and* the firmware binary to decode anything.

Other key strategies (weakest → strongest):

| Source | Notes |
|---|---|
| Compile-time constant in VM | Extractable from VM binary, simple |
| Derived from Wren version | `MAJOR<<16 \| MINOR<<8 \| PATCH` — effectively public |
| Host-provided via `WrenConfiguration` | Strongest — decoupled from VM source |

### Encoding function

XOR each byte against a value derived from the key, the entry index, and the
byte position within the entry. This ensures identical strings at different
positions encode differently, defeating simple pattern matching:

```c
static uint8_t obfuscationByte(uint32_t key, int nameIdx, int charIdx) {
  return (uint8_t)((key ^ (nameIdx * 31) ^ (charIdx * 17)) & 0xff);
}
```

### Serialization (encode)

```c
write_u16(count);
for (int i = 0; i < count; i++) {
  write_u16(entry[i].length);
  for (int j = 0; j < entry[i].length; j++) {
    uint8_t b = entry[i].bytes[j];
    if (key != 0) b ^= obfuscationByte(key, i, j);
    write_u8(b);
  }
}
```

### Deserialization (decode)

XOR is its own inverse — the decode path is identical:

```c
uint16_t count = read_u16();
for (int i = 0; i < count; i++) {
  uint16_t len = read_u16();
  for (int j = 0; j < len; j++) {
    uint8_t b = read_u8();
    if (key != 0) b ^= obfuscationByte(key, i, j);
    buf[j] = b;
  }
}
```

No extra passes needed — decode happens inline during the normal deserialization
walk described in `bytecode-deserialize.md`.

---

## Coverage Summary

| Threat | Strip debug | XOR name tables | XOR + constants | Full encryption |
|---|---|---|---|---|
| `strings -` on file | Partial | ✅ | ✅ | ✅ |
| Casual disassembly | ❌ | Harder | Harder | ✅ |
| Determined reverse engineer | ❌ | ❌ | ❌ | Slows them down |
| Attacker with RAM dump | ❌ | ❌ | ❌ | ❌ |

For most embedded use cases, **strip debug + XOR name tables + host-provided key**
is the right trade-off: minimal complexity, no runtime overhead worth measuring,
and the binary no longer reveals your application's logic at a glance.
