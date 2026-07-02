# Why wrenc Needs a Full WrenVM

`wrenc` cannot be a pure offline compiler that reads source and writes bytes
with no VM involved. The compiler (`wren_compiler.c`) is entangled with
`WrenVM*` in four distinct ways. Here's each one with the actual code.

---

## 1. All heap allocation goes through the VM

The compiler allocates live heap objects constantly while it works — not at the
end, but mid-parse, as it encounters each literal and function body:

```c
// String literal encountered during lexing:
parser->next.value = wrenNewStringLength(parser->vm, start, length);

// New function body begins:
compiler->fn = wrenNewFunction(parser->vm, parser->module, maxSlots);

// Deduplication map for constants:
compiler->constants = wrenNewMap(compiler->parser->vm);

// Class attribute map:
compiler->attributes = wrenNewMap(parser->vm);
```

These are real GC-managed heap objects. The VM's allocator (`wrenReallocate`),
GC gray list, and root stack are all involved. There is no way to allocate
`ObjFn`, `ObjString`, or `ObjMap` without a live VM.

---

## 2. Constants are GC-rooted against the VM during compilation

Whenever a new constant is added to a function, it's immediately pushed onto
the VM's root stack to keep the GC from collecting it before the `ObjFn` that
owns it is finished:

```c
// compiler.c:510
if (IS_OBJ(constant)) wrenPushRoot(compiler->parser->vm, AS_OBJ(constant));
wrenValueBufferWrite(compiler->parser->vm, &compiler->fn->constants, constant);
if (IS_OBJ(constant)) wrenPopRoot(compiler->parser->vm);
```

The GC can fire at any allocation. Without the VM's root stack to temporarily
protect in-flight objects, the GC would collect constants that haven't yet
been stored in a reachable `ObjFn`.

---

## 3. Method symbols are registered in `vm->methodNames`

Every method call site calls `wrenSymbolTableEnsure` against the VM's global
method name table:

```c
// compiler.c:1855
int symbol = wrenSymbolTableEnsure(compiler->parser->vm,
    &compiler->parser->vm->methodNames, name, length);
```

This table is shared across all modules in the VM. The compiler doesn't just
*read* it — it *writes* to it, adding new method names as they appear in
source. No VM means no `methodNames` table.

---

## 4. The VM tracks the active compiler for GC marking

The VM holds a pointer to the currently-running compiler so the GC can mark
all in-flight compiler objects as reachable during a collection:

```c
// compiler.c:547  (initCompiler)
parser->vm->compiler = compiler;

// compiler.c:1700  (endCompiler — pops back to parent)
compiler->parser->vm->compiler = compiler->parent;
```

`wrenMarkCompiler` (called by the GC) walks this chain and grays every
`ObjFn`, `ObjMap`, and other object the compiler is currently building. Remove
the VM and you have no GC marking for the compiler's in-flight objects.

---

## What wrenc Actually Does

Since we can't detach the compiler from the VM, `wrenc` takes the simplest
correct approach:

1. **Spin up a normal `WrenVM`** with a minimal config (no output, no foreign
   methods needed)
2. **Call `wrenCompile`** exactly as `wrenInterpret` would — this produces a
   live `ObjFn` tree on the VM heap
3. **Call `wrenSerializeFn`** to walk that live tree and write it to a file
4. **Tear down the VM** — the heap is discarded, the file is what we keep

The VM is ephemeral — it exists only long enough to compile. On a dev machine
this is fast and cheap. On the target device (microcontroller), `wrenc` never
runs at all — only the loader does.

```
Dev machine:   source.wren → [wrenc: VM + compiler + serializer] → module.wrenc
Microcontroller:             module.wrenc → [deserializer + patcher] → ObjFn → run
```

This is the same model used by:
- **Lua** (`luac` spins up a Lua state to compile, writes `.luac`)
- **Python** (`compileall` runs the Python runtime to produce `.pyc`)
- **MicroPython** (`mpy-cross` is a stripped-down interpreter used as a compiler)

---

## Could the Compiler Ever Be Decoupled?

Theoretically, yes — if allocation were abstracted behind an interface, and
`methodNames` were passed as a parameter rather than read from `vm->`. It
would be a significant refactor (~28 call sites touching `vm` directly in the
compiler alone). The payoff would be a true offline compiler with no VM
dependency. For now, the spin-up-and-serialize approach costs nothing and
delivers the same result.
