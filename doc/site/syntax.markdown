^title Syntax

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

However, it's pretty common to have a method or function that just evaluates and
returns the result of a single expression. Some other languages use `=>` to
define these. Wren uses:

    :::wren
    { "single expression" }

If there is no newline after the `{` (or after the parameter list in a
[function](functions.html)), then the block may only contain a single
expression, and it automatically returns the result of it. It's exactly the same
as doing:

    :::wren
    {
      return "single expression"
    }

Statements are not allowed in this form (since they don't produce values), which
means nothing starting with `class`, `for`, `if`, `import`,  `return`,
`var`, or `while`. If you want a block that contains a single statement,
put a newline in there:

    :::wren
    {
      if (happy) {
        System.print("I'm feelin' it!")
      }
    }

Using an initial newline after the `{` does feel a little weird or magical, but
newlines are already significant in Wren, so it's not totally crazy. The nice
thing about this syntax as opposed to something like `=>` is that the *end* of
the block has an explicit delimiter. That helps when chaining:

    :::wren
    numbers.map {|n| n * 2 }.where {|n| n < 100 }

## Precedence and Associativity

We'll talk about Wren's different expression forms and what they mean in the
next few pages. But if you want to see how they interact with each other
grammatically, here's the whole table.

It shows which expressions have higher *precedence*&mdash;which ones bind more
tightly than others&mdash;and their *associativity*&mdash;how a series of the
same kind of expression is ordered. Wren mostly follows C, except that it fixes
[the bitwise operator mistake][mistake]. The full precedence table, from
tightest to loosest, is:

[mistake]: http://www.lysator.liu.se/c/dmr-on-or.html

<table class="precedence">
  <tbody>
    <tr>
      <th>Prec</th>
      <th>Operator</th>
      <th>Description</th>
      <th>Associates</th>
    </tr>
    <tr>
      <td>1</td>
      <td><code>()</code> <code>[]</code> <code>.</code></td>
      <td>Grouping, <a href="method-calls.html">Subscript, Method call</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>2</td>
      <td><code>-</code> <code>!</code> <code>~</code></td>
      <td><a href="method-calls.html#operators">Negate, Not, Complement</a></td>
      <td>Right</td>
    </tr>
    <tr>
      <td>3</td>
      <td><code>*</code> <code>/</code> <code>%</code></td>
      <td><a href="method-calls.html#operators">Multiply, Divide, Modulo</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>4</td>
      <td><code>+</code> <code>-</code></td>
      <td><a href="method-calls.html#operators">Add, Subtract</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>5</td>
      <td><code>..</code> <code>...</code></td>
      <td><a href="method-calls.html#operators">Inclusive range, Exclusive range</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>6</td>
      <td><code>&lt;&lt;</code> <code>&gt;&gt;</code></td>
      <td><a href="method-calls.html#operators">Left shift, Right shift</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>7</td>
      <td><code>&amp;</code></td>
      <td><a href="method-calls.html#operators">Bitwise and</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>8</td>
      <td><code>^</code></td>
      <td><a href="method-calls.html#operators">Bitwise xor</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>9</td>
      <td><code>|</code></td>
      <td><a href="method-calls.html#operators">Bitwise or</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>10</td>
      <td><code>&lt;</code> <code>&lt;=</code> <code>&gt;</code> <code>&gt;=</code></td>
      <td><a href="method-calls.html#operators">Comparison</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>11</td>
      <td><code>is</code></td>
      <td><a href="method-calls.html#operators">Type test</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>12</td>
      <td><code>==</code> <code>!=</code></td>
      <td><a href="method-calls.html#operators">Equals, Not equal</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>13</td>
      <td><code>&amp;&amp;</code></td>
      <td><a href="control-flow.html#logical-operators">Logical and</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>14</td>
      <td><code>||</code></td>
      <td><a href="control-flow.html#logical-operators">Logical or</a></td>
      <td>Left</td>
    </tr>
    <tr>
      <td>15</td>
      <td><code>?:</code></td>
      <td><a href="control-flow.html#the-conditional-operator-">Conditional</a></td>
      <td>Right</td>
    </tr>
    <tr>
      <td>16</td>
      <td><code>=</code></td>
      <td><a href="variables.html#assignment">Assignment</a>, <a href="method-calls.html#setters">Setter</a></td>
      <td>Right</td>
    </tr>
  </tbody>
</table>

<br><hr>
<a class="right" href="values.html">Values &rarr;</a>
<a href="getting-started.html">&larr; Getting Started</a>
