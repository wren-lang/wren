^title Performance

Even though most benchmarks aren't worth the pixels they're printed on, people
seem to like them, so here's a few:

<h3>Method Call</h3>
<table class="chart">
  <tr>
    <th>wren</th><td><div class="chart-bar wren" style="width: 14%;">0.12s&nbsp;</div></td>
  </tr>
  <tr>
    <th>luajit (-joff)</th><td><div class="chart-bar" style="width: 18%;">0.16s&nbsp;</div></td>
  </tr>
  <tr>
    <th>ruby</th><td><div class="chart-bar" style="width: 23%;">0.20s&nbsp;</div></td>
  </tr>
  <tr>
    <th>lua</th><td><div class="chart-bar" style="width: 41%;">0.35s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python3</th><td><div class="chart-bar" style="width: 91%;">0.78s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python</th><td><div class="chart-bar" style="width: 100%;">0.85s&nbsp;</div></td>
  </tr>
</table>
<h3>DeltaBlue</h3>
<table class="chart">
  <tr>
    <th>wren</th><td><div class="chart-bar wren" style="width: 22%;">0.13s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python3</th><td><div class="chart-bar" style="width: 83%;">0.48s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python</th><td><div class="chart-bar" style="width: 100%;">0.57s&nbsp;</div></td>
  </tr>
</table>
<h3>Binary Trees</h3>
<table class="chart">
  <tr>
    <th>luajit (-joff)</th><td><div class="chart-bar" style="width: 20%;">0.11s&nbsp;</div></td>
  </tr>
  <tr>
    <th>wren</th><td><div class="chart-bar wren" style="width: 41%;">0.22s&nbsp;</div></td>
  </tr>
  <tr>
    <th>ruby</th><td><div class="chart-bar" style="width: 46%;">0.24s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python</th><td><div class="chart-bar" style="width: 71%;">0.37s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python3</th><td><div class="chart-bar" style="width: 73%;">0.38s&nbsp;</div></td>
  </tr>
  <tr>
    <th>lua</th><td><div class="chart-bar" style="width: 100%;">0.52s&nbsp;</div></td>
  </tr>
</table>
<h3>Recursive Fibonacci</h3>
<table class="chart">
  <tr>
    <th>luajit (-joff)</th><td><div class="chart-bar" style="width: 17%;">0.10s&nbsp;</div></td>
  </tr>
  <tr>
    <th>wren</th><td><div class="chart-bar wren" style="width: 35%;">0.20s&nbsp;</div></td>
  </tr>
  <tr>
    <th>ruby</th><td><div class="chart-bar" style="width: 39%;">0.22s&nbsp;</div></td>
  </tr>
  <tr>
    <th>lua</th><td><div class="chart-bar" style="width: 49%;">0.28s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python</th><td><div class="chart-bar" style="width: 90%;">0.51s&nbsp;</div></td>
  </tr>
  <tr>
    <th>python3</th><td><div class="chart-bar" style="width: 100%;">0.57s&nbsp;</div></td>
  </tr>
</table>

**Shorter bars are better.** Each benchmark is run ten times and the best time
is kept. It only measures the time taken to execute the benchmarked code
itself, not interpreter startup.

These were run on my MacBook Pro 2.3 GHz Intel Core i7 with 16 GB of 1,600 MHz
DDR3 RAM. Tested against Lua 5.2.3, LuaJIT 2.0.2, Python 2.7.5, Python 3.3.4,
ruby 2.0.0p247. LuaJIT is run with the JIT *disabled* (i.e. in bytecode
interpreter mode) since I want to support platforms where JIT-compilation is
disallowed. LuaJIT with the JIT enabled is *much* faster than all of the other
languages benchmarked, including Wren, because Mike Pall is a robot from the
future.

The benchmark harness and programs are
[here](https://github.com/wren-lang/wren/tree/master/test/benchmark).

## Why is Wren fast?

Languages come in four rough performance buckets, from slowest to fastest:

1.  Tree-walk interpreters: Ruby 1.8.7 and earlier, Io, that
    interpreter you wrote for a class in college.

2.  Bytecode interpreters: CPython,
    Ruby 1.9 and later, Lua, early JavaScript VMs.

3.  JIT compiled dynamically typed languages: Modern JavaScript VMs,
    LuaJIT, PyPy, some Lisp/Scheme implementations.

4.  Statically typed languages: C, C++, Java, C#, Haskell, etc.

Most languages in the first bucket aren't suitable for production use. (Servers
are one exception, because you can throw more hardware at a slow language
there.) Languages in the second bucket are fast enough for many use cases, even
on client hardware, as the success of the listed languages shows. Languages in
the third bucket are quite fast, but their implementations are breathtakingly
complex, often rivaling that of compilers for statically-typed languages.

Wren is in the second bucket. If you want a simple implementation that's fast
enough for real use, this is the sweet spot. In addition, Wren has a few tricks
up its sleeve:

### A compact value representation

A core piece of a dynamic language implementation is the data structure used
for variables. It needs to be able to store (or reference) a value of any type,
while also being as compact as possible. Wren uses a technique called *[NaN
tagging][]* for this.

[nan tagging]: http://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations

All values are stored internally in Wren as small, eight-byte double-precision
floats. Since that is also Wren's number type, in order to do arithmetic, no
conversion is needed before the "raw" number can be accessed: a value holding a
number *is* a valid double. This keeps arithmetic fast.

To store values of other types, it turns out there's a ton of unused bits in a
NaN double. You can stuff a pointer for heap-allocated objects, with room left
over for special values like `true`, `false`, and `null`. This means numbers,
bools, and null are unboxed. It also means an entire value is only eight bytes,
the native word size on 64-bit machines. Smaller = faster when you take into
account CPU caching and the cost of passing values around.

### Fixed object layout

Most dynamic languages treat objects as loose bags of named properties. You can
freely add and remove properties from an object after you've created it.
Languages like Lua and JavaScript don't even have a well-defined concept of a
"type" of object.

Wren is strictly class-based. Every object is an instance of a class. Classes
in turn have a well-defined declarative syntax, and cannot be imperatively
modified. In addition, fields in Wren are private to the class&mdash;they can
only be accessed from methods defined directly on that class.

Put all of that together and it means you can determine at *compile* time
exactly how many fields an object has and what they are. In other languages,
when you create an object, you allocate some initial memory for it, but that
may have to be reallocated multiple times as fields are added and the object
grows. Wren just does a single allocation up front for exactly the right number
of fields.

Likewise, when you access a field in other languages, the interpreter has to
look it up by name in a hash table in the object, and then maybe walk its
inheritance chain if it can't find it. It must do this every time since fields
may be added freely. In Wren, field access is just accessing a slot in the
instance by an offset known at compile time: it's just adding a few pointers.

### Copy-down inheritance

When you call a method on an object, the method must be located. It could be
defined directly on the object's class, or it may be inheriting it from some
superclass. This means that in the worst case, you may have to walk the
inheritance chain to find it.

Advanced implementations do very smart things to optimize this, but it's made
more difficult by the mutable nature of the underlying language: if you can add
new methods to existing classes freely or change the inheritance hierarchy, the
lookup for a given method may actually change over time. You have to check for
that which costs CPU cycles.

Wren's inheritance hierarchy is static and fixed at class definition time. This
means that we can copy down all inherited methods in the subclass when it's
created since we know those will never change. Method dispatch then just
requires locating the method in the class of the receiver.

### Method signatures

Wren supports overloading by arity using its concept of [signatures]. This makes
the language more expressive, but also faster. When a method is called, we look
it up on the receiver's class. If we succeed in finding it, we also know it has
the right number of parameters.

This lets Wren avoid the extra checking most languages need to do at runtime to
handle too few or too many arguments being passed to a method. In Wren, it's not
*syntactically* possible to call a method with the wrong number of arguments.

[signatures]: method-calls.html#signature

### Computed gotos

On compilers that support it, Wren's core bytecode interpreter loop uses
something called [*computed gotos*][goto]. The hot core of a bytecode
interpreter is effectively a giant `switch` on the instruction being executed.

[goto]: http://eli.thegreenplace.net/2012/07/12/computed-goto-for-efficient-dispatch-tables/

Doing that using an actual `switch` confounds the CPU's [branch
predictor][]&mdash;there is basically a single branch point for the entire
interpreter. That quickly saturates the predictor and it just gets confused and
fails to predict anything, which leads to more CPU stalls and pipeline flushes.

[branch predictor]: http://en.wikipedia.org/wiki/Branch_predictor

Using computed gotos gives you a separate branch point at the end of each
instruction. Each gets its own branch prediction, which often succeeds since
some instruction pairs are more common than others. In my rough testing, this
makes a 5-10% performance difference.

### A single-pass compiler

Compile time is a relatively small component of a language's performance: code
only has to be compiled once but a given line of code may be run many times.
However, fast compilation helps with *startup* speed&mdash;the time it takes to
get anything up and running. For that, Wren's compiler is quite fast.

It's modeled after Lua's compiler. Instead of tokenizing and then parsing to
create a bunch of AST structures which are then consumed and deallocated by
later phases, it emits code directly during parsing. This means it does minimal
memory allocation during a parse and has very little overhead.

## Why don't other languages do this?

Most of Wren's performance comes from language design decisions. While it's
dynamically *typed* and *dispatched*, classes are relatively statically
*defined*. That makes a lot of things much easier. Other languages have a much
more mutable object model, and cannot change that without breaking lots of
existing code.

Wren's closest sibling, by far, is Lua. Lua is more dynamic than Wren which
makes its job harder. Lua also tries very hard to be compatible across a wide
range of hardware and compilers. If you have a C89 compiler for it, odds are
very good that you can run Lua on it.

Wren cares about compatibility, but it requires C99 or C++98 and IEEE double
precision floats. That may exclude some edge case hardware, but makes things
like NaN tagging, computed gotos, and some other tricks possible.

<script src="script.js"></script>
