# The Root ObjFn — Module as a Tree

## How the Compiler Builds the Tree

The compiler is a **stack of `Compiler` structs**, each owning an `ObjFn` being
built. They're linked via `compiler->parent`. When a nested function or method
finishes compiling, `endCompiler` fires (compiler.c:1679):

```c
if (compiler->parent != NULL) {
  int constant = addConstant(compiler->parent, OBJ_VAL(compiler->fn));
  emitShortArg(compiler->parent, CODE_CLOSURE, constant);
}
```

The finished `ObjFn` is **stuffed into the parent's constant table**, and a
`CODE_CLOSURE` instruction is emitted in the parent's bytecode to say: "at
runtime, wrap constant[N] in a closure and push it onto the stack."

The root compiler has `parent == NULL` — it never gets inserted into anything.
`wrenCompile` returns it directly as the module's entry point. Everything else
in the module hangs off it transitively through constant tables.

## The Whole Module Is One ObjFn

`wrenCompile` returns a single `ObjFn` named `"(script)"`. Executing it
**is** running the module — it defines classes, binds methods, initializes
module variables, fires imports. None of that is pre-built structure; it's all
imperative bytecode that happens to run at module load time.

## Concrete Example

```wren
class Greeter {
  greet(name) {
    System.print("Hello, " + name)
  }

  static farewell(name) {
    System.print("Bye, " + name)
  }
}
```

Compiled tree:

```
ObjFn "(script)"                        ← root, returned by wrenCompile()
│
│  constants:
│    [0]  ObjFn "Greeter.greet(_)"
│    │      arity:      1
│    │      constants:
│    │        [0] "Hello, "
│    │      bytecode:
│    │        LOAD_MODULE_VAR  0        (System)
│    │        CONSTANT         0        "Hello, "
│    │        LOAD_LOCAL       1        (name arg)
│    │        CALL_1          [+(_)]    → "Hello, " + name
│    │        CALL_1          [print(_)]
│    │        RETURN
│    │        END
│    │
│    [1]  ObjFn "Greeter.farewell(_)"
│    │      arity:      1
│    │      constants:
│    │        [0] "Bye, "
│    │      bytecode:  (same shape as greet)
│    │
│    [2]  "Greeter"                     ← class name string constant
│
│  bytecode:
│    LOAD_MODULE_VAR  ?                 (Object — implicit superclass)
│    CLASS            0                 (0 fields)
│    CLOSURE          0                 → wraps constant[0]  (greet)
│    METHOD_INSTANCE [greet(_)]
│    CLOSURE          1                 → wraps constant[1]  (farewell)
│    METHOD_STATIC   [farewell(_)]
│    END_MODULE
│    RETURN
│    END
```

The root `ObjFn` does not *contain* a `Greeter` class — it contains the
**instructions to build one** at runtime. `CLASS` allocates the class object,
`CLOSURE` wraps each method body, and `METHOD_INSTANCE`/`METHOD_STATIC` bind
them. This is why you can't serialize a live class — but you *can* serialize
the bytecode that constructs it.

## Why This Is Good for Serialization

The `ObjFn` tree has two useful properties:

**1. No cycles.** A child `ObjFn` never references its parent. The tree is a
proper DAG, so a recursive-descent serializer needs no cycle detection. Write
constants depth-first, then bytecode.

**2. Self-contained.** Every function body the module needs lives somewhere in
the constant table tree. There is no separate method registry, class table, or
symbol pool that needs independent serialization. The tree *is* the module.

Serializing the root `ObjFn` recursively captures the entire compiled module.

## Nesting Can Go Arbitrarily Deep

Functions defined inside methods, closures inside functions — each level is
just another `ObjFn` in a constant table. The serializer doesn't need to know
the depth; it just follows `TAG_FN` entries recursively.

```wren
var add = Fn.new {|a|
  Fn.new {|b| a + b }   // ← ObjFn inside ObjFn inside ObjFn "(script)"
}
```

```
ObjFn "(script)"
  constants:
    [0] ObjFn "<block>"          ← outer Fn.new block
          constants:
            [0] ObjFn "<block>"  ← inner Fn.new block
                  bytecode: LOAD_UPVALUE 0 (a), LOAD_LOCAL 1 (b), CALL_1 [+(_)]
          bytecode: CLOSURE 0, RETURN, END
  bytecode: CLOSURE 0, STORE_MODULE_VAR 0 (add), ...
```
