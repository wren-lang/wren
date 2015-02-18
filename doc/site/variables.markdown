^title Variables
^category language

Variables are named slots for storing values. You can define a new variable in
Wren using a `var` statement, like so:

    :::dart
    var a = 1 + 2

This creates a new variable `a` in the current scope and initializes it with
the result of the expression following the `=`. Once a variable has been
defined, it can be accessed by name as you would expect.

    :::dart
    var animal = "Slow Loris"
    IO.print(animal) // Prints "Slow Loris".

## Scope

Wren has true block scope: a variable exists from the point where it is defined
until the end of the [block](syntax.html#blocks) where that definition appears.

    :::dart
    {
      IO.print(a) // ERROR! a doesn't exist yet.
      var a = 123
      IO.print(a) // "123"
    }
    IO.print(a) // ERROR! a doesn't exist anymore.

Variables defined at the top level of a script are *top-level* and are visible
to the [module](modules.html) system. All other variables are *local*.
Declaring a variable in an inner scope with the same name as an outer one is
called *shadowing* and is not an error (although it's not something you likely
intend to do much).

    :::dart
    var a = "outer"
    {
      var a = "inner"
      IO.print(a) // Prints "inner".
    }
    IO.print(a) // Prints "outer".

Declaring a variable with the same name in the *same* scope *is* an error.

    :::dart
    var a = "hi"
    var a = "again" // ERROR!

## Assignment

After a variable has been declared, you can assign to it using `=`:

    :::dart
    var a = 123
    a = 234

An assignment walks up the scope stack to find where the named variable is
declared. It's an error to assign to a variable that isn't defined. Wren
doesn't roll with implicit variable definition.

When used in a larger expression, an assignment expression evaluates to the
assigned value.

    :::dart
    var a = "before"
    IO.print(a = "after") // Prints "after".

**TODO: Top-level names.**
