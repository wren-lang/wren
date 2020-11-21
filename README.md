## Wren is a small, fast, class-based concurrent scripting language

Think Smalltalk in a Lua-sized package with a dash of Erlang and wrapped up in
a familiar, modern [syntax][].

```dart
System.print("Hello, world!")

class Wren {
  flyTo(city) {
    System.print("Flying to %(city)")
  }
}

var adjectives = Fiber.new {
  ["small", "clean", "fast"].each {|word| Fiber.yield(word) }
}

while (!adjectives.isDone) System.print(adjectives.call())
```

 *  **Wren is small.** The VM implementation is under [4,000 semicolons][src].
    You can skim the whole thing in an afternoon. It's *small*, but not
    *dense*. It is readable and [lovingly-commented][nan].

 *  **Wren is fast.** A fast single-pass compiler to tight bytecode, and a
    compact object representation help Wren [compete with other dynamic
    languages][perf].

 *  **Wren is class-based.** There are lots of scripting languages out there,
    but many have unusual or non-existent object models. Wren places
    [classes][] front and center.

 *  **Wren is concurrent.** Lightweight [fibers][] are core to the execution
    model and let you organize your program into an army of communicating
    coroutines.

 *  **Wren is a scripting language.** Wren is intended for embedding in
    applications. It has no dependencies, a small standard library,
    and [an easy-to-use C API][embedding]. It compiles cleanly as C99, C++98
    or anything later.

If you like the sound of this, [let's get started][started]. You can even try
it [in your browser][browser]! Excited? Well, come on and [get
involved][contribute]!

[![Build Status](https://travis-ci.org/wren-lang/wren.svg?branch=main)](https://travis-ci.org/wren-lang/wren)

[syntax]: http://wren.io/syntax.html
[src]: https://github.com/wren-lang/wren/tree/main/src
[nan]: https://github.com/wren-lang/wren/blob/93dac9132773c5bc0bbe92df5ccbff14da9d25a6/src/vm/wren_value.h#L486-L541
[perf]: http://wren.io/performance.html
[classes]: http://wren.io/classes.html
[fibers]: http://wren.io/concurrency.html
[embedding]: http://wren.io/embedding/
[started]: http://wren.io/getting-started.html
[browser]: http://ppvk.github.io/wren-nest/
[contribute]: http://wren.io/contributing.html
