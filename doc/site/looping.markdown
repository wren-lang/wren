^title Looping

It's hard to write a useful program without executing some chunk of code repeatedly. To do that in Wren, you use looping statements. There are two in Wren, and they should be familiar if you've used other imperative languages.

## While statements

A `while` statement executes a chunk of code as long as a condition continues to hold. For example:

    :::wren
    var n = 27
    while (n != 1) {
      if (n % 2 == 0) {
        n = n / 2
      } else {
        n = 3 * n + 1
      }
    }

This evaluates the expression `n != 1`. If it is [truthy](flow-control.html), then it executes the body: `n = 3 * n + 1`. After that, it loops back to the top, and evaluates the condition again. It keeps doing this as long as the condition is evaluates to something truthy.

The condition for a while loop can be any expression, and must be surrounded by parentheses. The body of the loop is usually a curly block but can also be a single statement:

    :::wren
    var n = 27
    while (n != 1) if (n % 2 == 0) n = n / 2 else n = 3 * n + 1

## For statements

While statements are useful when you want to loop indefinitely or according to some complex condition. But in most cases, you're looping through a [list](lists.html) or some other "sequence" object, or you're terating through a range of numbers. That's what `for` is for. It looks like this:

    for (beatle in ["george", "john", "paul", "ringo"]) {
      IO.write(beatle)
    }

A `for` loop has three components:

1. A *variable name* to bind. In the example, that's `beatle`. Wren will create a new variable with that name whose scope is the body of the loop.

2. A *sequence expression*. This determines what you're looping over. It gets evaluated *once* before the body of the loop. In this case, it's a list literal, but it can be any expression.

3. A *body*. This is a curly block or a single statement. It gets executed once for each iteration of the loop.

### The iterator protocol

It's important to be able to use looping constructs over user-defined sequence-like objects and not just built-in lists. To make that happen, the semantics of a `for` are defined in terms of an "iterator protocol". The loop itself doesn't know anything about lists or numbers, it just knows how to call two particular methods on the object that resulted from evaluating the sequence expression.

It works like this. First Wren evaluates the sequence expression and stores it in a hidden variable. In our example, it will just be the list object. It also creates a hidden "iterator" variable an initializes it to `null`.

At the beginning of each iteration, it calls `iterate()` on the sequence, and passes in the iterator. So in the first iteration, it always passes in `null`. The sequence's job is to take that iterator and advance it to the next element in the sequence (or, in the case where it's `null`, to advance it to the *first* element). It then returns either the new iterator, or `false` to indicate that there are no more elements.

If `false` is returned, Wren exits out of the loop and we're done. If anything else is returned, that means that we have advanced to a new valid element. To get that, Wren then calls `iteratorValue()` on the sequence and passes in that iterator value that it just got from the sequence. The sequence uses that to look up and return the appropriate element.

In other words, from Wren's perspective, the above loop looks something like this:

    {
      var iter_
      var seq_ = ["george", "john", "paul", "ringo"]
      while (iter_ = seq_.iterate(iter_)) {
        var beatle = seq_.iteratorValue(iter_)
        IO.write(beatle)
      }
    }

The built-in list type implements `iterate()` and `iteratorValue()` to walk over the list elements. You can implement the same methods in your classes to make your own types iterable.

### Numeric ranges

That just leaves iterating over numbers. Often you want to do something a fixed number of times, or with one of each of a range of consecutive numbers. In Wren, that looks like this:

    for (i in 1..100) {
      IO.write(i)
    }

This looks like some special range support in the `for` statement, but it's actually just the iterator protocol all over again. The `..` is a "range" operator. Like all other operators in Wren, it's just syntax for a method call. In this case, we're calling `..` on `1` and passing in `100`. The above example could just as well be written:

    var nums = 1..100
    for (i in nums) IO.write(i)

The number class implements the `..` method and returns a `Range` object. This is just a simple data structure that tracks a minimum and maximum (here `1` and `100`). The range class implements `iterate()` and `iteratorValue()` to generate a series of consecutive numbers.

If you don't want to execute the body of the loop for the last value, you can use `...` instead to get a half-open interval:

    for (i in 1...100) {
      IO.write(i)
    }

This will print up to `99`, but not `100`.

## Break statements
