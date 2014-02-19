^title Syntax

Wren's syntax is designed to be familiar to people coming from C-like languages while being as simple and expressive as possible within that framework.

Scripts are stored in plain text files with a `.wren` file extension. Wren does
not compile ahead of time: programs are run directly from source, from top to
bottom like a typical scripting language. (Internally, programs are compiled to
bytecode for efficiency, but that's an implementation detail).

## Comments

Line comments start with `//` and end at the end of the line:

    :::dart
    // This is a comment.

Block comments start with `/*` and end with `*/`. They can span multiple lines
or be within a single one. Unlike C, block comments can nest in Wren:

    :::dart
    /* This is /* a nested */ comment. */

## Reserved Words

Some people like to see all of the reserved words in a programming language in
one lump. If you're one of those folks, here you go:

    :::dart
    break class else false fn for if in is
    null return static this true var while

## Statement terminators

Officially, statements are terminated by a semicolon (`;`) like in other
languages in the C tradition. However, Wren treats newlines as equivalent
to a semicolon whenever it makes sense. In practice, this means you almost
never write `;` unless you want to cram a bunch of statements on one line.

    :::dart
    // Two statements:
    IO.write("hi")
    IO.write("bye")

Sometimes, though, a statement doesn't fit on a single line and treating the
newline as a semicolon would trip things up. To handle that, Wren has a very
simple rule: It ignores a newline following any token that can't end a
statement. Specifically, that means any of these:

    :::dart
    ( [ { . , * / % + - | || & && ! ~ = < > <= >= == !=
    class else if is static var while

Everywhere else, a newline is treated just like a `;`. Note that this is a very
different system from how JavaScript handles semicolons. If you've been burned
there, don't worry, you should be fine here.

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

**TODO: Move this somewhere else:*

### The `is` operator

The `is` keyword can be used as an infix operator in expression. It performs a
type test. The left operand is an object and the right operand is a class. It
evaluates to `true` if the object is an instance of the class (or one of its
subclasses).

**TODO: blocks, assignment, maps**
