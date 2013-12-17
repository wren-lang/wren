^title Performance

Languages come in four rough performance buckets, from slowest to fastest:

1.  Tree-walk interpreters: Ruby 1.8.7 and earlier, Io, that
    interpreter you wrote for a class in college.

2.  Bytecode interpreters: CPython,
    Ruby 1.9 and later, Lua, early JavaScript VMs.

3.  JIT compilers for dynamically typed languages: Modern JavaScript VMs,
    LuaJIT, PyPy, some Lisp/Scheme implementations.

4.  Statically compiled statically typed languages: C, C++, Java, C#, Haskell,
    etc.

Most languages in the first bucket aren't suitable for production use. (Servers are one exception, because you can always throw more hardware at a slow language there.) Languages in the second bucket are fast enough for many use cases, even on client hardware, as the success of the listed languages shows. Languages in the third bucket are quite fast, but their implementations are breathtakingly complex, often rivalling that of compilers for statically-typed languages.

## Why is Wren fast?

Wren is in the second bucket. If you want to have a simple implementation but be fast enough for real use, that's the natural home. Within that bucket, Wren's performance is quite competitive despite being much younger and with a much smaller, simpler codebase. What's the trick?

There are a few things Wren has to give it a leg up:

### A compact value representation

A core piece of a dynamic language implementation is the data structure used for variables. It needs to be able to store (or reference) a value of any type, while also being as compact as possible. Wren uses a technique called *[NaN tagging][]* for this.

[nan tagging]: http://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations

All values are stored internally in Wren as small, eight byte double-precision floats. Since that is also Wren's number type, in order to do arithmetic, no conversion is needed before the "raw" number can be accessed: a value holding a number *is* a valid double. This keeps arithmetic fast.

To store values of other types, it turns out there's a ton of unused bits in a "NaN" double. There's room in there for a pointer as well as some other stuff. For simple values like `true`, `false`, and `null`, Wren uses special bit patterns and stores them directly in the value. For other objects like strings that are heap allocated, Wren stores the pointer in there.

This means numbers, bools, and null are unboxed. It also means an entire value is only eight bytes, the native word size on 64-bit machines. Smaller = faster when you take into account CPU caching and the cost of passing values around.

### Fixed object layout

Most dynamic languages treat objects as loose bags of named properties. You can freely add and remove properties from an object after you've created it. Languages like Lua and JavaScript don't even have a well-defined concept of a "type" of object.

Wren is strictly class-based. Every object is an instance of a class. Classes in turn have a well-defined declarative syntax, and cannot be imperatively modified. In addition, fields in Wren are always class-private: they can only be accessed from methods defined directly on that class.

Put all of that together and it means you can determine at *compile* time exactly how many fields an object has and what they are. In other languages, when you create an object, you allocate some initial memory for it, but that may have to be reallocated multiple times as fields are added and the object grows. Wren just does a single allocation up front for exactly the right number of fields.

Likewise, when you access a field in other languages, the interpreter has to look it up by name in a hash table in the object, and then maybe walk its inheritance chain if it can't find it. It must do this every time since fields may be added freely. In Wren, field access is just accessing a slot in the instance by an offset known at compile time: it's just adding a few pointers.

### Copy down inheritance

When you call a method on an object, the method must be located. It could be defined directly on the object's class, or it may be inheriting it from some superclass. This means that in the worst case, you may have to walk the inheritance chain to find it.

Advanced implementations do very smart things to optimize this, but it's made more difficult by the mutable nature of the underlying language: if you can add new methods to existing classes freely or change the inheritance hierarchy, the lookup for a given method may actually change over time. You have to add guards to check for that, which cost CPU cycles.

Wren's inheritance hierarchy is static and fixed at class declaration time. This means that we can copy down all inherited methods in the subclass when it's created since we know those will never change. That means method dispatch just requires locating the method in the class of the receiver.

### Computed gotos

On compilers that support it, Wren's core bytecode interpreter loop will use something called [*computed gotos*](http://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables/). The hot core of a bytecode interpreter is effectively a giant `switch` on the instruction being executed.

Doing that using an actual `switch` wreaks havoc with the CPU's branch predictor: there is basically a single branch point for the entire interpreter. That quickly saturates the predictor and it just gets confused and fails to predict anything, which leads to more CPU stalls and pipeline flushes.

Using computed gotos gives you a separate branch point at the end of each instruction. Each gets its own branch prediction, which will often succeed since some instruction pairs are more common than others. In my rough testing, this made a 5-10% performance difference.

### A single-pass compiler

Compile time is a relatively small component of a language's performance: code only has to be compiled once but a given line of code may be run many many times. Still, Wren's compiler is quite fast.

It's modeled after Lua's compiler. Instead of tokenizing and then parsing to create a bunch AST structures which are then consumed and deallocated by later phases, it emits code directly during parsing. This means it does almost no memory allocation during a parse and has very little overhead.

## Why don't other languages do this?

Most of Wren's performance comes from language design decisions. While it's dynamically *typed* and *dispatched*, classes are relatively statically *defined*. That makes a lot of things much easier. Other languages have a much more mutable object model, and cannot change that without breaking lots of existing code.

Wren's closest sibling, by far, is Lua. Lua is more dynamic than Wren which makes its job harder. Lua also tries very hard to be compatible across a wide range of hardware and compilers. If you have a C89 compiler for it, odds are very good that you can run Lua on it.

Wren cares about compatibility, but it requires C99 and IEEE double precision floats. That may exclude some edge case hardware, but makes things like NaN tagging, computed gotos, and some other tricks possible.

## Do you have benchmarks to prove this?

Benchmarks are somewhere between an art and a carvinal game. They can easily be manipulated to show what you want. But, yes, there are several benchmarks in the repo.

**TODO: chart**
