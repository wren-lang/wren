^title Expressions
^category language

Wren's syntax is based on C so if you're familiar with that (or any of the
plethora of other languages based on it) you should be right at home. Since
Wren is heavily object-oriented, you'll notice that most of the different
expression forms are just different ways of invoking methods.

## Literals

Literals produce objects of built-in types. The primitive [value](values.html)
types&mdash;numbers, strings and friends&mdash;have literal forms as do the
built in collections: [lists](lists.html) and [maps](maps.html).

[Functions](functions.html) do not have standalone a literal form. Instead,
they are created by passing a [block
argument](functions.html#block-arguments) to a method.

## Identifiers

Names in expressions come in a few flavors. A name that starts with an
underscore denotes a [field](classes.html#fields), a piece of data stored in an
instance of a [class](classes.html). All other names refer to
[variables](variables.html).

## Method calls

Wren is object-oriented, so most code consists of method calls. Most of them
look like so:

    :::dart
    IO.print("hello")
    items.add("another")
    items.insert(1, "value")

You have a *receiver* expression followed by a `.`, then a name and an argument
list in parentheses. Arguments are separated by commas. Methods that do not
take any arguments omit the `()`:

    :::dart
    text.length

These are special "getters" or "accessors" in other languages. In Wren, they're
just method calls.

If the last (or only) argument to a method call is a
[function](functions.html), it may be passed as a [block
argument](functions.html#block-arguments):

    :::dart
    blondie.callMeAt(867, 5309) {
      IO.print("This is the body!")
    }

Semantically, all method calls work like so:

1. Evaluate the receiver and arguments.
2. Look up the method on the receiver's class.
3. Invoke it, passing in the arguments.

## This

The special `this` keyword works sort of like a variable, but has special
behavior. It always refers to the instance whose method is currently being
executed. This lets you invoke methods on "yourself".

It's an error to refer to `this` outside of a method. However, it's perfectly
fine to use it inside a function contained in a method. When you do, `this`
still refers to the instance whose method is being called.

This is unlike Lua and JavaScript which can "forget" `this` when you create a
callback inside a method. Wren does what you want here and retains the
reference to the original object. (In technical terms, a function's closure
includes `this`.)

## Super

Sometimes you want to invoke a method on yourself, but only methods defined in
one of your [superclasses](classes.html#inheritance). You typically do this in
an overridden method when you want to access the original method being
overridden.

To do that, you can use the special `super` keyword as the receiver in a method
call:

    :::dart
    class Base {
      method {
        IO.print("base method")
      }
    }

    class Derived is Base {
      method {
        super.method // Prints "base method".
      }
    }

You can also use `super` without a method name inside a constructor to invoke a
base class constructor:

    :::dart
    class Base {
      new(arg) {
        IO.print("base constructor got ", arg)
      }
    }

    class Derived is Base {
      new {
        super("value") // Prints "base constructor got value".
      }
    }

**TODO: constructors**

## Operators

Wren has most of the same operators you know and love with the same precedence
and associativity. Wren has three prefix operators:

    :::dart
    ! ~ -

They are just method calls on their operand without any other arguments. An
expression like `!possible` means "call the `!` method on `possible`".

We have a few other operators to play with. The remaining ones are
infix&mdash;they have operators on either side. They are:

    :::dart
    == != < > <= >= .. ... | & + - * / %

Like prefix operators, they are all funny ways of writing method calls. The
left operand is the receiver, and the right operand gets passed to it. So
`a + b` is semantically interpreted as "invoke the `+` method on `a`, passing
it `b`".

Most of these are probably familiar already. The `..` and `...` operators are
"range" operators. The number type implements those to create a
[range](values.html#ranges) object, but they are just regular operators.

Since operators are just method calls, this means Wren supports "operator
overloading" (though "operator over-*riding*" is more accurate). This can be
really useful when the operator is natural for what a class represents, but can
lead to mind-crushingly unreadable code when used recklessly. There's a reason
punctuation represents profanity in comic strips.

## Assignment

The `=` operator is used to *assign* or store a value somewhere. The right-hand
side can be any expression. If the left-hand side is an
[identifier](#identifiers), then the value of the right operand is stored in
the referenced [variable](variables.html) or [field](classes.html#fields).

The left-hand side may also be a method call, like:

    :::dart
    point.x = 123

In this case, the entire expression is a single "setter" method call. The above
example invokes the `x=` setter on the `point` object, and passing in `123`.
Sort of like `point.x=(123)`.

Since these are just regular method calls, you can define whatever setters you
like in your classes. However, you cannot change the behavior of *simple*
assignment. If the left-hand side is a variable name or field, an assignment
expression will always just store the value there.

## Subscript operators

Most languages use square brackets (`[]`) for working with collection-like
objects. For example:

    :::dart
    list[0]    // Gets the first item in a list.
    map["key"] // Gets the value associated with "key".

You know the refrain by now. In Wren, these are just method calls that a class
may define. Subscript operators may take multiple arguments, which is useful
for things like multi-dimensional arrays:

    :::dart
    matrix[3, 5]

Subscripts may also be used on the left-hand side of an assignment:

    :::dart
    list[0] = "item"
    map["key"] = "value"

Again, these are just method calls. The last example is equivalent to invoking
the `[]=` method on `map`, passing in `"key"` and `"value"`.

## Logical operators

The `&&` and `||` operators are not like the other infix operators. They work
more like [control flow](control-flow.html) structures than operators because
they conditionally execute some code&mdash;they short-circuit. Depending on the
value of the left-hand side, the right-hand operand expression may or may not
be evaluated. Because of this, they cannot be overloaded and they behavior is
fixed.

A `&&` ("logical and") expression evaluates the left-hand argument. If it's
[false](control-flow.html#truth), it returns that value. Otherwise it evaluates
and returns the right-hand argument.

    :::dart
    IO.print(false && 1)  // false
    IO.print(1 && 2)      // 2

An `||` ("logical or") expression is reversed. If the left-hand argument is
[true](control-flow.html#truth), it's returned, otherwise the right-hand
argument is evaluated and returned:

    :::dart
    IO.print(false || 1)  // 1
    IO.print(1 || 2)      // 1

## The conditional operator `?:`

Also known as the "ternary" operator since it takes three arguments, Wren has
the little "if statement in the form of an expression" you know and love from C
and its bretheren.

    :::dart
    IO.print(1 != 2 ? "math is sane" : "math is not sane!")

It takes a condition expression, followed by `?`, followed by a then
expression, a `:`, then an else expression. Just like `if`, it evaluates the
condition. If true, it evaluates (and returns) the then expression. Otherwise
it does the else expression.

## The `is` operator

Wren has one last expression form. You can use the `is` keyword like an infix
operator. It performs a type test. The left operand is an object and the right
operand is a class. It evaluates to `true` if the object is an instance of the
class (or one of its subclasses).

    :::dart
    123 is Num     // true
    "s" is Num     // false
    null is String // false
    [] is List     // true
    [] is Sequence // true

## Precedence

When you mix these all together, you need to worry about
*precedence*&mdash;which operators bind more tightly than others. Wren mostly
follows C, except that it fixes the bitwise operator mistake. The full
precedence table, from lowest to highest, is:

    :::dart
    =           // Assignment.
    && || ?:    // Logic.
    is          // Type test.
    == !=       // Equality.
    < > <= >=   // Comparison.
    .. ...      // Range.
    | &         // Bitwise.
    + -         // Terms.
    * / %       // Factors.
    - ~ !       // Unary.
    . []        // Call.
