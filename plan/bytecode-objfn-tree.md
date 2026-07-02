# ObjFn Tree Notes

This document collects background on how Wren represents a compiled module as a
tree of functions.

## Why It Matters

The first-pass bytecode feature can stay relatively small if it serializes the
compiled function tree the VM already builds.

## Scope for V1

- A compiled module can be treated as the root function plus nested functions
  in its constant tables.
- The serializer should follow that tree recursively.
- The loader should rebuild the same shape in VM memory.

## Module Variables Are Not Part Of The Tree

`ObjFn` and its constant table only capture code and literal data. Top-level
variable storage lives on `ObjModule`, not on any `ObjFn`, and
`LOAD_MODULE_VAR`/`STORE_MODULE_VAR` reference it by plain positional slot
index.

Before a module is compiled, the VM copies every core-module variable
(`System`, `Object`, `Fn`, `List`, `Map`, etc.) into the new module's leading
slots. Only variables declared after that point are "this module's own"
variables. The bytecode has no way to tell the two apart — a slot index is
just a slot index — so the serializer must record where that boundary was,
and the loader must recreate the same leading core slots from its own live
core module before laying the deserialized `ObjFn` tree on top. See
`bytecode-format.md` for the resulting payload shape.

## Not A Full Design Doc

This file is background material, not the primary implementation spec.

If later work needs a deeper object-model redesign, that should be documented
separately.
