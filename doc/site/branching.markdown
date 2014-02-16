^title Branching

*Control flow* is used to determine which chunks of code are executed and how many times. Expressions and statements for deciding whether or not to execute some code are called *branching* and are covered here. To execute something more than once, you'll want [*looping*](looping.html).

## Truthiness

Branching is conditional on the value of some expression. We take the entire universe of possible values and divide them into two buckets: some we consider "true" and the rest are "false". If the expression results in a value in the true bucket, we branch one way. Otherwise, we go the other way.

Obviously, the boolean `true` is in the "true" bucket and `false` is in "false", but what about values of other types? The choice is ultimately arbitrary, and different languages have different rules. Wren's rules follow Ruby:

  * The boolean value `false` is false.
  * The null value `null` is false.
  * Everything else is true.

This means `0`, empty strings, and empty collections are all considered "true" values.

## If statements

The simplest branching statement, `if` lets you conditionally skip a chunk of code. It looks like this:

    :::wren
    if (ready) IO.write("go!")

That evaluates the parenthesized expression after `if`. If it's true, then the statement after the condition is evaluated. Otherwise it is skipped. Instead of a statement, you can have a block:

    :::wren
    if (ready) {
      IO.write("getSet")
      IO.write("go!")
    }

You may also provide an `else` branch. It will be executed if the condition is false:

    :::wren
    if (ready) IO.write("go!") else IO.write("not ready!")

And, of course, it can take a block too:

    if (ready) {
      IO.write("go!")
    } else {
      IO.write("not ready!")
    }

## The logical operators `&&` and `||`

The `&&` and `||` operators are lumped here under branching because they conditionally execute some code&mdash;they short-circuit. Both of them are infix operators, and, depending on the value of the left-hand side, the right-hand operand expression may or may not be evaluated.

An `&&` ("logical and") expression evaluates the left-hand argument. If it's falsey, it returns that value. Otherwise it evaluates and returns the right-hand argument.

    :::wren
    IO.write(false && 1)  // false
    IO.write(1 && 2)      // 2

An `||` ("logical or") expression is reversed. If the left-hand argument is truthy, it's returned, otherwise the right-hand argument is evaluated and returned:

    :::wren
    IO.write(false || 1)  // 1
    IO.write(1 || 2)      // 1

**TODO: Conditional operator.**
