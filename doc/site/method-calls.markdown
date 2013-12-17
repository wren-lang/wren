^title Method Calls

Wren is object-oriented, so most code consists of method calls. They look like
this:

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

These are special "getters" or "accessors" in other languages. In Wren, they're
just methods.

## Arity

Unlike most dynamic languages, the number of arguments to a method is part of
its call signature. Methods with different signatures are distinct from each
other. In technical terms, this means you can overload by *arity*.

In normal human terms, it means you can overload by number of parameters. These
are calls to two different methods:

    items.add("one arg")
    items.add("first", "second")

Instead of having a single `add` method where you have to check for "undefined"
or missing arguments, Wren just treats them as different methods that you can
implement separately.

## Prefix Operators

Wren has mostly the same operators you know and love from C, with the same
precedence and associativity. These operators are prefix (they come before
their operand):

    :::wren
    ! ~ -

Semantically, these operators are just method calls on their operand. An
expression like `!possible` just means "call the `!` on `possible`".

### Infix Operators

These operators are infix (they have operands on either side):

    :::wren
    == !=
    < > <= >=
    | &
    + -
    * / %

Like prefix operators, they are just a funny way of writing a method call. The
left operand is the receiver, and the right operand gets passed to it. So
`a + b` is semantically interpreted as "invoke the `+` method on `a`, passing
it `b`".

### The `is` operator

The `is` keyword can be used as an infix operator in expression. It performs a
type test. The left operand is an object and the right operand is a class. It
evaluates to `true` if the object is an instance of the class (or one of its
subclasses).
