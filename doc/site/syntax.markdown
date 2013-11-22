^title Syntax

Wren's syntax is designed to be familiar to people coming from C-like languages while being as simple and expressive as possible within that framework.

## Comments

Line comments start with `//` and end at the end of the line:

    :::wren
    // This is a comment.

Block comments start with `/*` and end with `*/`. They can span multiple lines or be within a single one. Unlike C, block comments can nest in Wren:

    :::wren
    /* This is /* a nested */ comment. */

## Literals

Wren currently supports a few atomic types: booleans, numbers, strings, and the null type. Numbers are double-precision floating point values, and strings are text. Wren doesn't support a lot of fancy formats for them yet, just the basic:

    :::wren
    0
    1234
    -432.1
    "a string"
    "another string"
    "supported escapes: \" \n \\"

Wren also has a couple of special values:

* `null` indicates the absence of a value.
* `this` inside a method refers to the method's receiver.
* `true` and `false` are boolean values.

## Variables

Variables are named references to values. Naming rules mostly follow C. They start with a letter and can contain letters, digits, and underscores (`_`). Some examples:

    :::wren
    foo
    Bar
    bestFriends4eva
    under_score

Variables can be declared using `var`:

    :::wren
    var pie = "pumpkin"

**TODO: Scoping**

## Method calls

Wren is a deeply object-oriented language, so most code consists of method calls. They look pretty familiar:

    :::wren
    io.write("hello")
    items.add("another")
    items.insert(1, "value")

You have a *receiver* on the left, followed by a `.`, then a name and an argument list in parentheses. Semantically, a method call works like this:

1. Look up the class of the receiver.
2. Look up the method on it by name.
3. Invoke the method.

Methods that do not take any arguments leave off the `()`:

    :::wren
    text.length

These are special "getters" or "accessors" in other languages. In Wren, they're just methods. Unlike most dynamic languages, the number of arguments to a method is part of its *name*. In technical terms, this means you can overload by *arity*. Basically, it means that these are calls to two different methods:

    items.add("one arg")
    items.add("first", "second")

## Operators

Wren has mostly the same operators you know and love from C, with the same precedence and associativity. These operators are prefix (they come before their operand):

    :::wren
    ! ~ -

Semantically, they are just method calls on their operand. When you see `!possible`, it's effectively the same as `possible.!` (though Wren does *not* allow that syntax).

These operators are infix (they have operands on either side):

    :::wren
    =
    || &&
    is
    == !=
    < > <= >=
    | &
    + -
    * / %

The `is` operator is used for type tests. The left operand is an object and the right operand is a class. It evaluates to `true` if the object is an instance of the class (or one of its subclasses).

The `||` and `&&` are logical operators. Like in C, they are basically flow-control constructs. A `||` expression will only evaluate the right operand if the left-hand side evaluates to something non-false-y. Likewise, `&&` only evaluates the right operand if the left evaluates to something false-y.

In Wren, the only false value is the boolean value `false`. Everything else is considered "true".

All other infix operators are just syntactic sugar for method calls. The left operand is the receiver, and the right is passed to it as an argument. So `a + b` is semantically `a.+(b)`. The built-in types implement these methods to do what you (hopefully) expect.

**TODO: assignment, functions, lists, maps, flow control, whitespace and newlines**
