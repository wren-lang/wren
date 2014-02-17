^title Method Calls

**TODO: Refactor `method-calls` and `classes` into using and creating classes.**

Wren is object-oriented, so most code consists of method calls. Most of them
look like so:

    :::wren
    IO.write("hello")
    items.add("another")
    items.insert(1, "value")

You have a *receiver* on the left, followed by a `.`, then a name and an
argument list in parentheses. Semantically, a method call works like this:

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

## Setters

Modifying a public property of some object looks like you expect:

    :::wren
    point.x = 123

You can probably guess by now, but again this is just another special syntax
for a regular method call. The semantics for the above are "invoke the `x=`
method on `point`, passing `123` as an argument."

## Operators

Wren has most of the same operators you know and love from C and friends, with
the same precedence and associativity. They are listed here because they are
just a special syntax for regular method calls.

Wren has a few prefix operators:

    :::wren
    ! ~ -

They are just method calls on the operand without any other arguments. An
expression like `!possible` just means "call the `!` method on `possible`".

We have a few other operators to play with. The remaining ones are
infix&mdash;they have operators on either side. In order of increasing
precedence, they are:

    :::wren
    == !=
    < > <= >=
    .. ...
    | &
    + -
    * / %

Like prefix operators, they are just a funny way of writing a method call. The
left operand is the receiver, and the right operand gets passed to it. So
`a + b` is semantically interpreted as "invoke the `+` method on `a`, passing
it `b`".

Most of these are probably familiar already. The `..` and `...` operators are
"range" operators. The number type implements those and returns a range object,
which can in turn be iterated over using a `for` [loop](looping.html).

## Subscript operators

Most languages use square brackets (`[]`) for working with collection-like
objects. For example:

    :::wren
    list.add["item"]
    map["key"] = "value"

You know the refrain by now. In Wren, these are just method calls. Subscript
operators can also be overloaded by arity, which is useful for things like
multi-dimensional arrays:

    :::wren
    table[3, 5] = "value"
