^title Welcome

Wren is a *small, clean, fast, class-based scripting language.* Think Smalltalk
in a Lua-sized package.

    :::java
    IO.print("Hello, world!")

    class Wren {
      adjectives = ["small", "clean", "fast"]
      languageType {
        "scripting"
      }
    }

 *  **Wren is small.** The codebase is and always will be under 5,000
    semicolons to keep the language and libraries small enough to fit in your
    head. You can skim [the whole thing][src] in one sitting.

 *  **Wren is clean.** The codebase is *small*, but not *dense*. It is readable
    and [lovingly-commented][nan]. It's written in warning-free standard C99.

 *  **Wren is fast.** A fast single-pass compiler, tight bytecode, and a
    compact object representation help Wren [compete with other dynamic
    languages][perf].

 *  **Wren is class-based.** There are lots of scripting languages out there,
    but many have unusual or non-existent object models. Wren places
    [classes][] front and center.

 *  **Wren is a scripting language.** Wren is intended for embedding in
    applications. It has no dependencies, a small standard library,
    and [an easy-to-use C API][usage].

[src]: https://github.com/munificent/wren/tree/master/src
[nan]: https://github.com/munificent/wren/blob/46c1ba92492e9257aba6418403161072d640cb29/src/wren_value.h#L378-L433
[perf]: performance.html
[classes]: classes.html
[usage]: usage.html
