# Deserialization — wrenDeserializeFn

Reconstructs a live `ObjFn` tree and `ObjModule` from a `.wrenc` binary, ready
to hand to the VM as if `wrenCompile` had just returned it.

## Inputs

- A byte buffer (or FILE*) produced by `wrenSerializeFn`
- `WrenVM* vm` — the loading VM (may be completely fresh)

## Outputs

- `ObjFn* fn` — the root function, ready to execute
- `ObjModule* module` — populated with variable names, wired into every `ObjFn`

Returns `NULL` on any format or version error.

## Steps

### 1. Read and validate header

```c
char magic[4];
read_bytes(magic, 4);
if (memcmp(magic, "WREN", 4) != 0) return error("not a wren bytecode file");

uint8_t major = read_u8();
uint8_t minor = read_u8();
uint8_t patch = read_u8();
if (major != WREN_VERSION_MAJOR ||
    minor != WREN_VERSION_MINOR ||
    patch != WREN_VERSION_PATCH) {
  return error("bytecode version mismatch");
}

uint8_t flags = read_u8();
bool hasDebug = flags & FLAG_HAS_DEBUG;
```

### 2. Read symbol name table → build remap[]

```c
uint16_t symCount = read_u16();
int* remap = allocate(symCount * sizeof(int));

for (int i = 0; i < symCount; i++) {
  uint16_t len = read_u16();
  char* name = read_bytes_alloc(len);

  // Ensure the name exists in this VM's global method table.
  // Adds it if new (safe — the VM will just never dispatch it if
  // no class defines a matching method).
  remap[i] = wrenSymbolTableEnsure(vm, &vm->methodNames, name, len);
}
```

`remap[localIdx]` now maps every local symbol index from the file to the
correct VM-global index in this VM.

### 3. Read module variable name table → allocate ObjModule

```c
ObjModule* module = wrenNewModule(vm, moduleName);

uint16_t varCount = read_u16();
for (int i = 0; i < varCount; i++) {
  uint16_t len = read_u16();
  char* name = read_bytes_alloc(len);
  // Reserve the slot — value will be populated when the script executes
  wrenDefineVariable(vm, module, name, len, NULL_VAL, NULL);
}
```

The module shell exists now. All deserialized `ObjFn`s will point to it.

### 4. Deserialize root ObjFn (recursive)

```c
ObjFn* root = readFn(vm, module, remap, hasDebug);
```

`readFn` is the core recursive function:

```c
static ObjFn* readFn(WrenVM* vm, ObjModule* module,
                     int* remap, bool hasDebug) {
  uint8_t maxSlots    = read_u8();
  uint8_t numUpvalues = read_u8();
  uint8_t arity       = read_u8();

  ObjFn* fn = wrenNewFunction(vm, module, maxSlots);
  fn->numUpvalues = numUpvalues;
  fn->arity = arity;

  // --- constants ---
  uint16_t constCount = read_u16();
  for (int i = 0; i < constCount; i++) {
    Value v = readConstant(vm, module, remap, hasDebug);
    wrenValueBufferWrite(vm, &fn->constants, v);
  }

  // --- bytecode ---
  uint16_t codeLen = read_u16();
  for (int i = 0; i < codeLen; i++) {
    wrenByteBufferWrite(vm, &fn->code, read_u8());
  }

  // --- debug ---
  if (hasDebug) {
    uint16_t nameLen = read_u16();
    char* name = read_bytes_alloc(nameLen);
    wrenFunctionBindName(vm, fn, name, nameLen);

    uint16_t lineCount = read_u16();
    for (int i = 0; i < lineCount; i++) {
      wrenIntBufferWrite(vm, &fn->debug->sourceLines, read_u16());
    }
  }

  return fn;
}
```

### 5. Reading constants

```c
static Value readConstant(WrenVM* vm, ObjModule* module,
                          int* remap, bool hasDebug) {
  uint8_t tag = read_u8();
  switch (tag) {
    case TAG_NULL: return NULL_VAL;
    case TAG_NUM:  return NUM_VAL(read_f64());
    case TAG_STR: {
      uint16_t len = read_u16();
      char* bytes = read_bytes_alloc(len);
      return OBJ_VAL(wrenNewStringLength(vm, bytes, len));
    }
    case TAG_FN:
      return OBJ_VAL(readFn(vm, module, remap, hasDebug));  // recurse
    default:
      return error("unknown constant tag");
  }
}
```

### 6. Patch bytecode — apply symbol remap

After `readFn` returns, walk the bytecode of every `ObjFn` in the tree and
replace local symbol indices with VM-global ones. This is a single linear pass
per function; nested functions are patched as they're deserialized (step 4
calls `readFn` recursively, so each `ObjFn` is patched before its parent
returns).

```c
static void patchSymbols(ObjFn* fn, int* remap) {
  int ip = 0;
  while (ip < fn->code.count) {
    Code op = (Code)fn->code.data[ip++];

    if (isCallOrMethod(op)) {
      uint16_t localIdx = (fn->code.data[ip] << 8) | fn->code.data[ip+1];
      uint16_t vmIdx    = (uint16_t)remap[localIdx];
      fn->code.data[ip]   = (vmIdx >> 8) & 0xff;
      fn->code.data[ip+1] =  vmIdx       & 0xff;
      ip += 2;
      if (isSuperOp(op)) ip += 2;  // skip superclass constant index
    } else {
      ip += getByteCountForArguments(fn->code.data,
                                     fn->constants.data, ip - 1) - 1;
    }
  }
}
```

Alternatively, patching can be done **inline during bytecode reading** in
`readFn` — read the local index from the buffer, write the remapped VM-global
index directly into `fn->code`. One less pass. Either approach works.

### 7. Return root

The root `ObjFn` is now equivalent to what `wrenCompile` would have produced
on this VM. Hand it to the VM's fiber/scheduler as normal.

## Error Handling

Any read that overflows the buffer, encounters an unknown tag, or fails a
version check should return `NULL` with an error message. The partially-built
`ObjFn` tree will be collected by the GC naturally — no manual cleanup needed
since all objects were allocated via the VM allocator and are already rooted.

## GC Safety

`wrenNewFunction`, `wrenNewStringLength`, etc. can trigger a GC cycle.
During deserialization, push roots as needed — or use the compiler's existing
`wrenPushRoot` / `wrenPopRoot` pattern around any allocation that isn't
immediately stored in a reachable object. The safest approach: root the
partially-built `ObjFn` before allocating its constants.
