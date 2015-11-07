^title Syntax
^category guide

Wren's syntax is designed to be familiar to people coming from C-like languages
while being a bit simpler and more streamlined.

Scripts are stored in plain text files with a `.wren` file extension. Wren does
not compile ahead of time: programs are run directly from source, from top to
bottom like a typical scripting language. (Internally, programs are compiled to
bytecode for [efficiency][], but that's an implementation detail.)

[efficiency]: performance.html

## Comments

Line comments start with `//` and end at the end of the line:

    :::wren
    // This is a comment.

Block comments start with `/*` and end with `*/`. They can span multiple lines:

    :::wren
    /* This
       is
       a
       multi-line
       comment. */

Unlike C, block comments can nest in Wren:

    :::wren
    /* This is /* a nested */ comment. */

This is handy because it lets you easily comment out an entire block of code,
even if the code already contains block comments.

## Reserved words

One way to get a quick feel for a language's style is to see what words it
reserves. Here's what Wren has:

    :::wren
    break class construct else false for foreign if import
    in is null return static super this true var while

## Identifiers

Naming rules are similar to other programming languages. Identifiers start with
a letter or underscore and may contain letters, digits, and underscores. Case
is sensitive.

    :::wren
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

    :::wren
    // Two statements:
    System.print("hi") // Newline.
    System.print("bye")

Sometimes, though, a statement doesn't fit on a single line and jamming a
newline in the middle would trip it up. To handle that, Wren has a very simple
rule: It ignores a newline following any token that can't end a statement.

    :::wren
    System.print( // Newline here is ignored.
        "hi")

In practice, this means you can put each statement on its own line and wrap
them across lines as needed without too much trouble.

## Blocks

Wren uses curly braces to define *blocks*. You can use a block anywhere a
statement is allowed, like in [control flow](control-flow.html) statements.
[Method](classes.html#methods) and [function](functions.html) bodies are also
blocks. For example, here we have a block for the then case, and a single
statement for the else:

    :::wren
    if (happy && knowIt) {
      hands.clap()
    } else System.print("sad")

Blocks have two similar but not identical forms. Typically, blocks contain a
series of statements like:

    :::wren
    {
      System.print("one")
      System.print("two")
      System.print("three")
    }

Blocks of this form when used for method and function bodies automatically
return `null` after the block has completed. If you want to return a different
value, you need an explicit `return` statement.

However, it's pretty common to have a method or function that just evaluates
and returns the result of a single expression. For that, Wren has a more
compact notation:

    :::wren
    { "single expression" }

If there is no newline after the `{` (or after the parameter list in a
[function](functions.html)), then the block may only contain a single
expression, and it automatically returns the result of it. It's exactly the
same as doing:

    :::wren
    {
      return "single expression"
    }

<a class="right" href="values.html">Values &rarr;</a>
<a href="getting-started.html">&larr; Getting Started</a>