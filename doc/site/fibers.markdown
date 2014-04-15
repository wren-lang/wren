^title Fibers

Fibers are a key part of Wren. They form its execution model, its concurrency story and take the place of exceptions in [error handling](error-handling.html).

They are a bit like threads except they are *cooperatively* scheduled. That means Wren won't stop running a fiber and switch to another until you tell to. You don't have to worry about context switches at random times and all of the headaches those cause.

Fibers in Wren are dramatically more expressive than generators in Python and C#. They are on par with coroutines in Lua (though Wren supports both symmetric and asymmetric coroutines while Lua only directly has the latter).

Fibers are lightweight and efficient in Wren. They are scheduled entirely by Wren, so they don't use OS thread resources, or require heavyweight context switches. They just need a bit of memory for their stacks. A fiber will get garbage collected like any other object when not referenced any more, so you can create them freely.

My goal is to keep them lightweight enough that you can safely, for example, have a separate fiber for each entity in a game. Wren should be able to handle thousands of them without any trouble. For example, when you run Wren in interactive mode, it creates a new fiber for every line of code you type in.

## Creating fibers

All Wren code runs within the context of a fiber. When you first start a Wren script, a main fiber is created for you automatically. You can spawn new fibers using the `Fiber` class's constructor:

    :::dart
    var fiber = new Fiber {
      IO.print("This runs in a separate fiber.")
    }

Creating a fiber does not immediately run it. It's just a first class bundle of code sitting there waiting to be activated, a bit like a [function](functions.html).

## Running fibers

Once you've created a fiber, you can run it (which suspends the current fiber) by calling its `run` method:

    :::dart
    fiber.run

The run fiber will then execute its code until it reaches the end of its body or until it passes control to another fiber. If it reaches the end of its body, it's considered *done*:

    :::dart
    var fiber = new Fiber { IO.print("Hi") }
    fiber.isDone // false
    fiber.run
    fiber.isDone // true

When it finishes, it will automatically resume the fiber that ran it. This works like coroutines in Lua. It's a runtime error to try to run a fiber that is already done.

## Yielding

The main difference between fibers and functions is that a fiber can be suspended in the middle of its operation and then resumed later. Running another fiber is one way to suspend a fiber, but that's more or less the same as one function calling another.

Things get interesting when a fiber *yields*. A yielded fiber passes control *back* to the fiber that ran it, but *remembers where it is*. The next time the fiber is run, it picks up right where it left off and keeps going.

You can make a fiber yield by calling the static `yield` method on `Fiber`:

    :::dart
    var fiber = new Fiber {
      IO.print("fiber 1")
      Fiber.yield
      IO.print("fiber 2")
    }

    IO.print("main 1")
    fiber.run
    IO.print("main 2")
    fiber.run
    IO.print("main 3")

This program prints:

    :::text
    main 1
    fiber 1
    main 2
    fiber 2
    main 3

Note that even though this program has *concurrency*, it's still *deterministic*. You can reason precisely about what it's doing and aren't at the mercy of a thread scheduler playing Russian roulette with your code.

**TODO: show example that can't do with generators**

**TODO: pass values to run, yield**

## Symmetric concurrency

**TODO**
