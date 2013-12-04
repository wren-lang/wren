^title Flow Control

## Truthiness

Flow control is about evaluating an expression and then choosing which code to execute based on whether or not the result is "true". That's easy if the value happens to be a boolean, but what if it's some other type?

Languages handle this by having a set of rules for what values of any given type are "true" and will cause a condition to be met. Wren calls this "truthiness" and "falsiness". The rules are simple (and follow Ruby):

  * The boolean value `false` is falsey.
  * The null value `null` is falsey.
  * Everything else is truthy.

This means `0`, empty strings, and empty collections are all considered truthy values.

## If statements

The simplest flow control structure, `if` lets you conditionally skip a chunk of code. It looks like this:

    :::wren
    if (ready) io.write("go!")

That will evaluate the parenthesized expression after `if`. If it's truthy, then the expression after the condition is evaluated. Otherwise it is skipped. Instead of an expression, you can have a block:

    :::wren
    if (ready) {
      io.write("getSet")
      io.write("go!")
    }

You may also provide an `else` branch. It will be evaluated if the condition is falsey:

    :::wren
    if (ready) io.write("go!") else io.write("not ready!")

And, of course, it can take a block too:

    if (ready) {
      io.write("go!")
    } else {
      io.write("not ready!")
    }

## The logical operators `&&` and `||`

The `&&` and `||` operators are lumped here under flow control because they conditionally execute some code&mdash;they short-circuit. Both of them are infix operators, and, depending on the value of the left-hand side, the right-hand operand expression may or may not be evaluated.

An `&&` ("logical and") expression evaluates the left-hand argument. If it's falsey, it returns that value. Otherwise it evaluates and returns the right-hand argument.

    :::wren
    io.write(false && 1)  // false
    io.write(1 && 2)      // 2

An `||` ("logical or") expression is reversed. If the left-hand argument is truthy, it's returned, otherwise the right-hand argument is evaluated and returned:

    :::wren
    io.write(false || 1)  // 1
    io.write(1 || 2)      // 1

## While statements

**TODO**

## For statements

**TODO**
