^title Concurrency

Lightweight concurrency is a key feature of Wren and it is expressed using
*fibers*. They control how all code is executed, and take the place of
exceptions in [error handling](error-handling.html).

Fibers are a bit like threads except they are *cooperatively* scheduled. That
means Wren doesn't pause one fiber and switch to another until you tell it to.
You don't have to worry about context switches at random times and all of the
headaches those cause.

Wren takes care of all of the fibers in the VM, so they don't use OS thread
resources, or require heavyweight context switches. Each just needs a bit of
memory for its stack. A fiber will get garbage collected like any other object
when not referenced any more, so you can create them freely.

They are lightweight enough that you can, for example, have a separate fiber for
each entity in a game. Wren can handle thousands of them without breaking a
sweat. For example, when you run Wren in interactive mode, it creates a new
fiber for every line of code you type in.

## Creating fibers

All Wren code runs within the context of a fiber. When you first start a Wren
script, a main fiber is created for you automatically. You can spawn new fibers
using the Fiber class's constructor:

    :::wren
    var fiber = Fiber.new {
      System.print("This runs in a separate fiber.")
    }

It takes a [function][] containing the code the fiber should execute. The
function can take zero or one parameter, but no more than that. Creating the
fiber does not immediately run it. It just wraps the function and sits there,
waiting to be activated.

[function]: functions.html

## Invoking fibers

Once you've created a fiber, you run it by calling its `call()` method:

    :::wren
    fiber.call()

This suspends the current fiber and executes the called one until it reaches the
end of its body or until it passes control to yet another fiber. If it reaches
the end of its body, it is considered *done*:

    :::wren
    var fiber = Fiber.new {
      System.print("It's alive!")
    }

    System.print(fiber.isDone) //> false
    fiber.call() //> It's alive!
    System.print(fiber.isDone) //> true

When a called fiber finishes, it automatically passes control *back* to the
fiber that called it. It's a runtime error to try to call a fiber that is
already done.

## Yielding

The main difference between fibers and functions is that a fiber can be
suspended in the middle of its operation and then resumed later. Calling
another fiber is one way to suspend a fiber, but that's more or less the same
as one function calling another.

Things get interesting when a fiber *yields*. A yielded fiber passes control
*back* to the fiber that ran it, but *remembers where it is*. The next time the
fiber is called, it picks up right where it left off and keeps going.

You make a fiber yield by calling the static `yield()` method on Fiber:

    :::wren
    var fiber = Fiber.new {
      System.print("Before yield")
      Fiber.yield()
      System.print("Resumed")
    }

    System.print("Before call") //> Before call
    fiber.call() //> Before yield
    System.print("Calling again") //> Calling again
    fiber.call() //> Resumed
    System.print("All done") //> All done

Note that even though this program uses *concurrency*, it is still
*deterministic*. You can reason precisely about what it's doing and aren't at
the mercy of a thread scheduler playing Russian roulette with your code.

## Passing values

Calling and yielding fibers is used for passing control, but it can also pass
*data*. When you call a fiber, you can optionally pass a value to it.

If you create a fiber using a function that takes a parameter, you can pass a
value to it through `call()`:

    :::wren
    var fiber = Fiber.new {|param|
      System.print(param)
    }

    fiber.call("Here you go") //> Here you go

If the fiber has yielded and is waiting to resume, the value you pass to call
becomes the return value of the `yield()` call when it resumes:

    :::wren
    var fiber = Fiber.new {|param|
      System.print(param)
      var result = Fiber.yield()
      System.print(result)
    }

    fiber.call("First") //> First
    fiber.call("Second") //> Second

Fibers can also pass values *back* when they yield. If you pass an argument to
`yield()`, that will become the return value of the `call()` that was used to
invoke the fiber:

    :::wren
    var fiber = Fiber.new {
      Fiber.yield("Reply")
    }

    System.print(fiber.call()) //> Reply

This is sort of like how a function call may return a value, except that a fiber
may return a whole sequence of values, one every time it yields.

## Full coroutines

What we've seen so far is very similar to what you can do with languages like
Python and C# that have *generators*. Those let you define a function call that
you can suspend and resume. When using the function, it appears like a sequence
you can iterate over.

Wren's fibers can do that, but they can do much more. Like Lua, they are full
*coroutines*&mdash;they can suspend from anywhere in the callstack. The function
you use to create a fiber can call a method that calls another method that calls
some third method which finally calls yield. When that happens, *all* of those
method calls &mdash; the entire callstack &mdash; gets suspended. For example:

    :::wren
    var fiber = Fiber.new {
      (1..10).each {|i|
        Fiber.yield(i)
      }
    }

Here, we're calling `yield()` from within a [function](functions.html) being
passed to the `each()` method. This works fine in Wren because that inner
`yield()` call will suspend the call to `each()` and the function passed to it
as a callback.

## Transferring control

Fibers have one more trick up their sleeves. When you execute a fiber using
`call()`, the fiber tracks which fiber it will return to when it yields. This
lets you build up a chain of fiber calls that will eventually unwind back to
the main fiber when all of the called ones yield or finish.

This is usually what you want. But if you're doing something low level, like
writing your own scheduler to manage a pool of fibers, you may not want to treat
them explicitly like a stack.

For rare cases like that, fibers also have a `transfer()` method. This switches
execution to the transferred fiber and "forgets" the fiber that was transferred
*from*. The previous one is suspended, leaving it in whatever state it was in.
You can resume the previous fiber by explicitly transferring back to it, or even
calling it. If you don't, execution stops when the last transferred fiber
returns.

Where `call()` and `yield()` are analogous to calling and returning from
functions, `transfer()` works more like an unstructured goto. It lets you freely
switch control between a number of fibers, all of which act as peers to one
another.

<br><hr>
<a class="right" href="error-handling.html">Error Handling &rarr;</a>
<a href="classes.html">&larr; Classes</a>
