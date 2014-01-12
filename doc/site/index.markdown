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

 *  **Wren is small.** The codebase stays under 5,000 semicolons to keep the
    language and libraries small enough to fit in your head. You can skim the
    whole thing in one sitting.

 *  **Wren is clean.** The codebase is *small*, but not *dense*. It is readable
    and lovingly-commented. It's written in warning-free standard C99.

 *  **Wren is fast.** Wren has a fast single-pass compiler, tight bytecode, and
    a compact object representation.

 *  **Wren is class-based.** There are lots of scripting languages out there,
    but many have unusual or non-existent object models. Wren places
    classes front and center.

 *  **Wren is a scripting language.** Wren is intended for embedding in
    applications. It has no dependencies, a small standard library,
    and an easy-to-use C API.
