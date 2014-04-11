^title Looping

It's hard to write a useful program without executing some chunk of code repeatedly. To do that, you use looping statements. There are two in Wren, and they should be familiar if you've used other imperative languages.

## While statements

A `while` statement executes a chunk of code as long as a condition continues to hold. For example:

    :::dart
    // Hailstone sequence.
    var n = 27
    while (n != 1) {
      if (n % 2 == 0) {
        n = n / 2
      } else {
        n = 3 * n + 1
      }
    }

This evaluates the expression `n != 1`. If it is [true](branching.html), then it executes the following body. After that, it loops back to the top, and evaluates the condition again. It keeps doing this as long as the condition evaluates to something true.

The condition for a while loop can be any expression, and must be surrounded by parentheses. The body of the loop is usually a curly block but can also be a single statement:

    :::dart
    var n = 27
    while (n != 1) if (n % 2 == 0) n = n / 2 else n = 3 * n + 1

## For statements

While statements are useful when you want to loop indefinitely or according to some complex condition. But in most cases, you're looping through a [list](lists.html), a series of numbers, or some other "sequence" object. That's what `for` is for. It looks like this:

    :::dart
    for (beatle in ["george", "john", "paul", "ringo"]) {
      IO.print(beatle)
    }

A `for` loop has three components:

1. A *variable name* to bind. In the example, that's `beatle`. Wren will create a new variable with that name whose scope is the body of the loop.

2. A *sequence expression*. This determines what you're looping over. It gets evaluated *once* before the body of the loop. In this case, it's a list literal, but it can be any expression.

3. A *body*. This is a curly block or a single statement. It gets executed once for each iteration of the loop.

## Break statements

Sometimes, right in the middle of a loop body, you decide you want to bail out and stop. To do that, you can use a `break` statement. It's just the `break` keyword all by itself. That will immediately exit out of the nearest enclosing `while` or `for` loop.

    :::dart
    for (i in [1, 2, 3, 4]) {
      IO.print(i)
      if (i == 3) break
    }

So this program will print the numbers from 1 to 3, but will not print 4.

## Numeric ranges

Lists are one common use for `for` loops, but sometimes you want to walk over a sequence of numbers, or loop a number of times. For that, you can use a *range* expression, like so:

    :::dart
    for (i in 1..100) {
      IO.print(i)
    }

This loops over the numbers from 1 to 100, including 100 itself. If you want to leave off the last value, use three dots instead of two:

    :::dart
    for (i in 1...100) {
      IO.print(i)
    }

This looks like some special "range" syntax in the `for` loop, but it's actually just a pair of operators. The `..` and `...` syntax are infix "range" operators. Like [other operators](method-calls.html), they are just special syntax for a regular method call. The number type implements them and returns instances of a `Range` class. That class in turn knows how to iterate over a series of numbers.

## The iterator protocol

Lists and ranges cover the two most common kinds of loops, but you should also be able to define your own sequences. To enable that, the semantics of a `for` are defined in terms of an "iterator protocol". The loop itself doesn't know anything about lists or ranges, it just knows how to call two particular methods on the object that resulted from evaluating the sequence expression.

When you write a loop like this:

    :::dart
    for (i in 1..100) {
      IO.print(i)
    }

Wren sees it something like this:

    :::dart
    var iter_ = null
    var seq_ = 1..100
    while (iter_ = seq_.iterate(iter_)) {
      var i = seq_.iteratorValue(iter_)
      IO.print(i)
    }

First, Wren evaluates the sequence expression and stores it in a hidden variable (written `seq_` in the example but in reality it doesn't have a name you can use). It also creates a hidden "iterator" variable and initializes it to `null`.

Each iteration, it calls `iterate()` on the sequence, passing in the current iterator value. (In the first iteration, it passes in `null`.) The sequence's job is to take that iterator and advance it to the next element in the sequence. (Or, in the case where the iterator is `null`, to advance it to the *first* element). It then returns either the new iterator, or `false` to indicate that there are no more elements.

If `false` is returned, Wren exits out of the loop and we're done. If anything else is returned, that means that we have advanced to a new valid element. To get that, Wren then calls `iteratorValue()` on the sequence and passes in the iterator value that it just got from calling `iterate()`. The sequence uses that to look up and return the appropriate element.

The built-in List and Range types implement `iterate()` and `iteratorValue()` to walk over their respective sequences. You can implement the same methods in your classes to make your own types iterable.
