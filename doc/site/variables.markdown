^title Variables

Variables are named slots for storing values. You can define a new variable in
Wren using a `var` statement, like so:

    :::wren
    var a = 1 + 2

This creates a new variable `a` in the current scope and initializes it with
the result of the expression following the `=`. Once a variable has been
defined, it can be accessed by name as you would expect.

    :::wren
    var animal = "Slow Loris"
    io.write(animal) // prints "Slow Loris"

## Scope

Wren has true block scope: a variable exists from the point where it is
defined until the end of the block where that definition appears.

    :::wren
    {
        io.write(a) // ERROR! a doesn't exist yet
        var a = 123
        io.write(a) // "123"
    }
    io.write(a) // ERROR! a doesn't exist anymore

Variables defined at the top level of a script are *global*. All other variables
are *local*. Declaring a variable in an inner scope with the same name as an
outer one is called *shadowing* and is not an error (although it's not
something you likely intend to do much).

    :::wren
    var a = "outer"
    {
        var a = "inner"
        io.write(a) // Prints "inner".
    }
    io.write(a) // Prints "outer".

Declaring a variable with the same name in the *same* scope *is* an error.

    :::wren
    var a = "hi"
    var a = "again" // ERROR!

## Assignment

After a variable has been declared, you can assign to it using `=`:

    var a = 123
    a = 234

An assignment walks up the scope stack to find where the named variable is
declared. It's an error to assign to a variable that isn't defined. Wren
doesn't roll with implicit variable definition.

When used in a larger expression, an assignment expression evaluates to the
assigned value.

    :::wren
    var a = "before"
    io.write(a = "after") // Prints "after".

**TODO: Forward references for globals, closures.**
