^title Fibers
^category types

Fibers are a key part of Wren. They form its execution model, its concurrency
story, and take the place of exceptions in [error
handling](error-handling.html).

They are a bit like threads except they are *cooperatively* scheduled. That
means Wren doesn't pause one fiber and switch to another until you tell it to.
You don't have to worry about context switches at random times and all of the
headaches those cause.

Fibers are managed entirely by Wren, so they don't use OS thread resources, or
require heavyweight context switches. They just need a bit of memory for their
stacks. A fiber will get garbage collected like any other object when not
referenced any more, so you can create them freely.

They are lightweight enough that you can, for example, have a separate fiber
for each entity in a game. Wren can handle thousands of them without any
trouble. For example, when you run Wren in interactive mode, it creates a new
fiber for every line of code you type in.

## Creating fibers

All Wren code runs within the context of a fiber. When you first start a Wren
script, a main fiber is created for you automatically. You can spawn new fibers
using the `Fiber` class's constructor:

    :::dart
    var fiber = Fiber.new {
      IO.print("This runs in a separate fiber.")
    }

Creating a fiber does not immediately run it. It's just a first class bundle of
code sitting there waiting to be activated, a bit like a
[function](functions.html).

## Invoking fibers

Once you've created a fiber, you can invoke it (which suspends the current
fiber) by calling its `call()` method:

    :::dart
    fiber.call()

The called fiber will execute its code until it reaches the end of its body or
until it passes control to another fiber. If it reaches the end of its body,
it's considered *done*:

    :::dart
    var fiber = Fiber.new { IO.print("Hi") }
    fiber.isDone // false
    fiber.call()
    fiber.isDone // true

When it finishes, it automatically resumes the fiber that called it. It's a
runtime error to try to call a fiber that is already done.

## Yielding

The main difference between fibers and functions is that a fiber can be
suspended in the middle of its operation and then resumed later. Calling
another fiber is one way to suspend a fiber, but that's more or less the same
as one function calling another.

Things get interesting when a fiber *yields*. A yielded fiber passes control
*back* to the fiber that ran it, but *remembers where it is*. The next time the
fiber is called, it picks up right where it left off and keeps going.

You can make a fiber yield by calling the static `yield()` method on `Fiber`:

    :::dart
    var fiber = Fiber.new {
      IO.print("fiber 1")
      Fiber.yield()
      IO.print("fiber 2")
    }

    IO.print("main 1")
    fiber.call()
    IO.print("main 2")
    fiber.call()
    IO.print("main 3")

This program prints:

    :::text
    main 1
    fiber 1
    main 2
    fiber 2
    main 3

Note that even though this program has *concurrency*, it's still
*deterministic*. You can reason precisely about what it's doing and aren't at
the mercy of a thread scheduler playing Russian roulette with your code.

## Passing values

Calling and yielding fibers is used for passing control, but it can also pass
*data*. When you call a fiber, you can optionally pass a value to it. If the
fiber has yielded and is waiting to resume, the value becomes the return value
of the `yield()` call:

    :::dart
    var fiber = Fiber.new {
      var result = Fiber.yield()
      IO.print(result)
    }

    fiber.call("discarded")
    fiber.call("sent")

This prints "sent". Note that the first value sent to the fiber through call is
ignored. That's because the fiber isn't waiting on a `yield()` call, so there's
no where for the sent value to go.

Fibers can also pass values *back* when they yield. If you pass an argument to
`yield()`, that will become the return value of the `call` that was used to
invoke the fiber:

    :::dart
    var fiber = Fiber.new {
      Fiber.yield("sent")
    }

    IO.print(fiber.call())

This also prints "sent".

## Full coroutines

What we've seen so far is very similar to what you can do with languages like
Python and C# that have *generators*. Those let you define a function call that
you can suspend and resume. When using the function, it appears like a sequence
you can iterate over.

Wren's fibers can do that, but they can do much more. Like Lua, they are full
*coroutines*&mdash;they can suspend from anywhere in the callstack. For
example:

    :::dart
    var fiber = Fiber.new {
      (1..10).map {|i|
        Fiber.yield(i)
      }
    }

Here, we're calling `yield()` from within a [function](functions.html) being
passed to the `map()` method. This works fine in Wren because that inner
`yield()` call will suspend the call to `map()` and the function passed to it
as a callback.

## Transferring control

Fibers have one more trick up their sleeves. When you execute a fiber using
`call()`, the fiber tracks which fiber it will return to when it yields. This
lets you build up a chain of fiber calls that will eventually unwind back to
the main fiber when all of the called ones yield or finish.

This works fine for most uses, but sometimes you want something a little more
freeform. For example, you may be creating a [state
machine](http://en.wikipedia.org/wiki/Finite-state_machine) where each state is
a fiber. When you switch from one state to the next, you *don't* want to build
an implicit stack of fibers to return to. There is no "returning" in this case.
You just want to *transfer* to the next fiber and forget about the previous one
entirely. (This is analogous to [tail call
elimination](http://en.wikipedia.org/wiki/Tail_call) for regular function
calls.)

To enable this, fibers also have a `run()` method. This begins executing that
fiber, and "forgets" the previous one. If the running fiber yields or ends, it
will transfer control back to the last *called* one. (If there are no called
fibers, it will end execution.)
