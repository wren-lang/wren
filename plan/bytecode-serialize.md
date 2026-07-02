# Serialization — wrenSerializeFn

Transforms a compiled `ObjFn` tree (the output of `wrenCompile`) plus its
module metadata into a flat binary stream suitable for writing to disk.

## Inputs

- `ObjFn* fn` — the root function returned by `wrenCompile`
- `ObjModule* module` — the module it belongs to (for variable names)
- `WrenVM* vm` — needed to resolve symbol names from `vm->methodNames`
- `bool includeDebug` — whether to write `FnDebug` (names + line numbers)

## Output

A byte buffer (or FILE*) in the format described in `bytecode-format.md`.

## Steps

### 1. Write header

```c
write_bytes("WREN", 4);
write_u8(WREN_VERSION_MAJOR);
write_u8(WREN_VERSION_MINOR);
write_u8(WREN_VERSION_PATCH);
write_u8(includeDebug ? FLAG_HAS_DEBUG : 0);
```

### 2. Collect all referenced method symbols

Walk the entire `ObjFn` tree **before writing anything else** — we need the
complete symbol set upfront to assign stable local indices.

```c
// Use a SymbolTable (already exists in Wren) as an ordered set.
// wrenSymbolTableFind returns -1 if absent; wrenSymbolTableEnsure adds if new.
SymbolTable localSymbols;   // local index → method name
wrenSymbolTableInit(&localSymbols);
collectSymbols(fn, vm, &localSymbols);  // recursive walk
```

`collectSymbols` walks every instruction. For each `CALL`, `SUPER`,
`METHOD_INSTANCE`, `METHOD_STATIC` it encounters:

```c
uint16_t vmIdx = READ_SHORT();
const char* name = vm->methodNames.data[vmIdx]->value;
int nameLen = vm->methodNames.data[vmIdx]->length;
wrenSymbolTableEnsure(NULL, &localSymbols, name, nameLen);
// adds if new, returns local index either way
```

For `TAG_FN` constants, recurse.

### 3. Write symbol name table

```c
write_u16(localSymbols.count);
for (int i = 0; i < localSymbols.count; i++) {
  ObjString* s = localSymbols.data[i];
  write_u16(s->length);
  write_bytes(s->value, s->length);
}
```

### 4. Write module variable name table

```c
write_u16(module->variableNames.count);
for (int i = 0; i < module->variableNames.count; i++) {
  ObjString* s = module->variableNames.data[i];
  write_u16(s->length);
  write_bytes(s->value, s->length);
}
```

### 5. Write root ObjFn (recursive)

```c
writeFn(fn, vm, &localSymbols, includeDebug);
```

`writeFn` is the core recursive function:

```c
static void writeFn(ObjFn* fn, WrenVM* vm,
                    SymbolTable* localSymbols, bool includeDebug) {
  write_u8(fn->maxSlots);
  write_u8(fn->numUpvalues);
  write_u8(fn->arity);

  // --- constants ---
  write_u16(fn->constants.count);
  for (int i = 0; i < fn->constants.count; i++) {
    writeConstant(fn->constants.data[i], vm, localSymbols, includeDebug);
  }

  // --- bytecode (with VM-global symbol IDs replaced by local indices) ---
  write_u16(fn->code.count);
  writeCodeWithRemappedSymbols(fn, vm, localSymbols);

  // --- debug ---
  if (includeDebug) {
    write_u16(fn->debug->nameLength);
    write_bytes(fn->debug->name, fn->debug->nameLength);
    write_u16(fn->code.count);  // one line entry per bytecode byte
    for (int i = 0; i < fn->code.count; i++) {
      write_u16(fn->debug->sourceLines.data[i]);
    }
  }
}
```

### 6. Writing constants

```c
static void writeConstant(Value v, WrenVM* vm,
                          SymbolTable* localSymbols, bool includeDebug) {
  if (IS_NULL(v))       { write_u8(TAG_NULL); return; }
  if (IS_NUM(v))        { write_u8(TAG_NUM);  write_f64(AS_NUM(v)); return; }
  if (IS_STRING(v))     {
    ObjString* s = AS_STRING(v);
    write_u8(TAG_STR);
    write_u16(s->length);
    write_bytes(s->value, s->length);
    return;
  }
  if (IS_FN(v))         { write_u8(TAG_FN); writeFn(AS_FN(v), ...); return; }
  UNREACHABLE();  // no other types appear in compile-time constant tables
}
```

### 7. Writing bytecode with symbol remapping

Copy `fn->code` byte by byte, but for every symbol-carrying instruction,
replace the 2-byte VM-global index with the 2-byte local index:

```c
static void writeCodeWithRemappedSymbols(ObjFn* fn, WrenVM* vm,
                                         SymbolTable* localSymbols) {
  int ip = 0;
  while (ip < fn->code.count) {
    Code op = fn->code.data[ip];
    write_u8(op);
    ip++;

    if (isCallOrMethod(op)) {
      // Read the VM-global symbol index
      uint16_t vmIdx = (fn->code.data[ip] << 8) | fn->code.data[ip+1];
      // Translate to local index
      const char* name = vm->methodNames.data[vmIdx]->value;
      uint16_t localIdx = wrenSymbolTableFind(localSymbols, name,
                                              strlen(name));
      write_u16(localIdx);
      ip += 2;

      // SUPER also has an extra 2-byte superclass constant index — copy as-is
      if (isSuperOp(op)) { write_u16(READ_SHORT()); ip += 2; }
    } else {
      // Copy remaining bytes for this instruction unchanged
      int width = getByteCountForArguments(fn->code.data,
                                           fn->constants.data, ip - 1);
      write_bytes(&fn->code.data[ip], width - 1);
      ip += width - 1;
    }
  }
}
```

## What We Do NOT Serialize

- `ObjClosure` — closures are runtime wrappers around `ObjFn`; the VM creates
  them when executing `CODE_CLOSURE`
- `ObjClass` — classes are built at runtime by `CODE_CLASS` / `CODE_METHOD_*`
- `fn->module` pointer — reconstructed at load time from the deserializer's
  freshly allocated `ObjModule`
- Any runtime heap object — instances, maps, lists, fibers, upvalue objects
