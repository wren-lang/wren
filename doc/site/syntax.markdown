^title Syntax
^category language

Wren's syntax is designed to be familiar to people coming from C-like languages while being as simple and expressive as possible within that framework.

Scripts are stored in plain text files with a `.wren` file extension. Wren does
not compile ahead of time: programs are run directly from source, from top to
bottom like a typical scripting language. (Internally, programs are compiled to
bytecode for efficiency, but that's an implementation detail.)

## Comments

Line comments start with `//` and end at the end of the line:

    :::dart
    // This is a comment.

Block comments start with `/*` and end with `*/`. They can span multiple lines
or be within a single one. Unlike C, block comments can nest in Wren:

    :::dart
    /* This is /* a nested */ comment. */

## Reserved words

Some people like to see all of the reserved words in a programming language in
one lump. If you're one of those folks, here you go:

    :::dart
    break class else for if in is return static var while

Wren also has a few predefined identifiers:

    :::dart
    false null this true

## Names

Identifiers are similar to other programming languages. They start with a letter or underscore and may contain letters, digits, and underscores. Case is sensitive.

    :::dart
    hi
    camelCase
    PascalCase
    _under_score
    abc123
    ALL_CAPS

Identifiers that start with underscore (`_`) are special in Wren. They are used to indicate fields in [classes](classes.html).

## Newlines

Newlines (`\n`) are meaningful in Wren. They are used to separate statements:

    :::dart
    // Two statements:
    IO.print("hi") // Newline.
    IO.print("bye")

Sometimes, though, a statement doesn't fit on a single line and jamming a
newline in the middle would trip it up. To handle that, Wren has a very
simple rule: It ignores a newline following any token that can't end a
statement.

    :::dart
    IO.print( // Newline here is ignored.
        "hi")

In practice, this means you can put each statement on its own line and wrap
them across lines as needed without too much trouble.

## Blocks

Wren uses curly braces to define *blocks*. Things like [control flow](branching.html) and [looping](looping.html) allow block bodies. [Method](method-calls.html) and [function](functions.html) bodies are also blocks. For example:

    :::dart
    if (happy && knowIt) {
      hands.clap
    } else IO.print("sad")

Here we have a block for the then case, and just a single expression for the else. Blocks have two similar but not identical forms. If a there is a newline after the opening `{`, then the body contains a series of statements:

    :::dart
    {
      IO.print("one")
      IO.print("two")
      IO.print("three")
    }

If there is no newline, the block may only contain a single expression:

    :::dart
    { "this is fine" }
    { while (this) "is an error" }

These are useful when defining method and function bodies. A normal block body implicitly returns `null`. If you want your method or function to return something different, you need an explicit `return` statement. However, a single-expression block with no newline after the `{` implicitly returns the result of that expression. This is a nice convenience for short methods and functions that just evaluate and return an expression.

**TODO: Move this somewhere else:**

## The `is` operator

The `is` keyword can be used as an infix operator in expression. It performs a
type test. The left operand is an object and the right operand is a class. It
evaluates to `true` if the object is an instance of the class (or one of its
subclasses).

**TODO: blocks, assignment, maps**
