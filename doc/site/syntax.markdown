^title Syntax
^category language

Wren's syntax is designed to be familiar to people coming from C-like languages
while being a bit simpler and more streamlined.

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
    break class else false for foreign if in is new
    null return static super this true var while

## Identifiers

Naming rules are similar to other programming languages. Identifiers start with
a letter or underscore and may contain letters, digits, and underscores. Case
is sensitive.

    :::dart
    hi
    camelCase
    PascalCase
    _under_score
    abc123
    ALL_CAPS

Identifiers that start with underscore (`_`) are special in Wren. They are used
to indicate [fields](classes.html#fields) in classes.

## Newlines

Newlines (`\n`) are meaningful in Wren. They are used to separate statements:

    :::dart
    // Two statements:
    IO.print("hi") // Newline.
    IO.print("bye")

Sometimes, though, a statement doesn't fit on a single line and jamming a
newline in the middle would trip it up. To handle that, Wren has a very simple
rule: It ignores a newline following any token that can't end a statement.

    :::dart
    IO.print( // Newline here is ignored.
        "hi")

In practice, this means you can put each statement on its own line and wrap
them across lines as needed without too much trouble.

## Blocks

Wren uses curly braces to define *blocks*. You can use a block anywhere a
statement is allowed, like in [control flow](control-flow.html) statements.
[Method](classes.html#methods) and [function](functions.html) bodies are also
blocks. For example, here we have a block for the then case, and a single
statement for the else:

    :::dart
    if (happy && knowIt) {
      hands.clap
    } else IO.print("sad")

Blocks have two similar but not identical forms. Typically, blocks contain a
series of statements like:

    :::dart
    {
      IO.print("one")
      IO.print("two")
      IO.print("three")
    }

Blocks of this form when used for method and function bodies automatically
return `null` after the block has completed. If you want to return a different
value, you need an explicit `return` statement.

However, it's pretty common to have a method or function that just evaluates
and returns the result of a single expression. For that, Wren has a more
compact notation:

    :::dart
    { "single expression" }

If there is no newline after the `{` (or after the parameter list in a of
[function](functions.html)), then the block may only contain a single
expression, and it automatically returns the result of it. It's exactly the
same as doing:

    :::dart
    {
        return "single expression"
    }
