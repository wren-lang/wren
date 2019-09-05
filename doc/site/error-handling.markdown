^title Error Handling

Errors come in a few fun flavors.

## Syntax errors

The first errors you're likely to run into are syntax errors. These include
simple bugs where your code doesn't follow the language's grammar, like:

    :::wren
    1 + * 2

Wren detects these errors as soon as it tries to read your code. When it hits
one, you get a friendly error message, like:

    :::text
    [main line 1] Error on '*': Unexpected token for expression.

Some slightly more "semantic" errors fall into this bucket too. Things like
using a variable that hasn't been defined, or declaring two variables with the
same name in the same scope. So if you do:

    :::wren
    var a = "once"
    var a = "twice"

Wren tells you:

    :::text
    [main line 2] Error on 'a': Top-level variable is already defined.

Note that it does this before it executes *any* code. Unlike some other
scripting languages, Wren tries to help you find your errors as soon as
possible when it can.

If it starts running your code, you can be sure you don't have any errors
related to syntax or variable scope.

## Runtime errors

Alas, just fixing all of the "compile-time" errors doesn't mean your code does
what you want. Your program may still have errors that can't be detected
statically. Since they can't be found until your code is run, they're called
"runtime" errors.

Most runtime errors come from the VM itself. They arise from code trying to
perform an operation that the VM can't do. The most common error is a "method
not found" one. If you call a method on an object and its class (and all of its
superclasses) don't define that method, there's nothing Wren can do:

    :::wren
    class Foo {
      construct new() {}
    }

    var foo = Foo.new()
    foo.someRandomMethod

If you run this, Wren will print:

    :::text
    Foo does not implement method 'someRandomMethod'.

Then it stops executing code. Unlike some other languages, Wren doesn't keep
plugging away after a runtime error has occurred. A runtime error implies
there's a bug in your code and it wants to draw your attention to it. To help
you out, it prints a stack trace showing where in the code the error occurred,
and all of the method calls that led to it.

Another common runtime error is passing an argument of the wrong type to a
method. For example, lists are indexed using a number. If you try to pass some
other type, it's an error:

    :::wren
    var list = ["a", "b", "c"]
    list["1"]

This exits with:

    :::text
    Subscript must be a number or a range.
    [main line 2] in (script)

These are the two most common kinds of runtime errors, but there are others.
Stuff like out of bounds errors on lists, calling a function with the wrong
number of arguments, etc.

## Handling runtime errors

Most of the time, runtime errors indicate a bug in your code and the best
solution is to fix the bug. However, sometimes it's useful to be able to handle
them at, uh, runtime.

To keep the language simpler, Wren does not have exception handling. Instead, it
takes advantage of [fibers][] for handling errors. When a runtime error occurs,
the current fiber is aborted. Normally, Wren will also abort any fibers that
invoked that one, all the way to the main fiber, and then exit the VM.

[fibers]: concurrency.html

However, you can run a fiber using the `try` method. If a runtime error occurs
in the called fiber, the error is captured and the `try` method returns the
error message as a string.

For example, if you run this program:

    :::wren
    var fiber = Fiber.new {
      123.badMethod
    }

    var error = fiber.try()
    System.print("Caught error: " + error)

It prints:

    :::text
    Caught error: Num does not implement method 'badMethod'.

The called fiber can no longer be used, but any other fibers can proceed as
usual. When a fiber has been aborted because of a runtime error, you can also
get the error from the fiber object. Continuing the above example:

    :::wren
    System.print(fiber.error)

This also prints:

    :::text
    Num does not implement method 'badMethod'.

If you have a chain of fiber calls and a runtime error occurs, it will walk the
chain looking for a `try` call, so this can also be used to capture runtime
errors generated in fibers that are invoked by the one you called `try` on.

## Creating runtime errors

Most runtime errors come from within the Wren VM, but you may want to be able
to cause your own runtime errors to occur. This can be done by calling the
`abort()` static method on `Fiber`:

    :::wren
    Fiber.abort("Something bad happened")

You must pass in an error message, and it must be a string.

If the provided message is `null`, no runtime error is raised.

## Failures

The last flavor of errors is the highest-level one. All of the above errors
indicate *bugs*&mdash;places where the code itself is incorrect. But some
errors indicate that the code simply couldn't accomplish its task for
unforeseeable reasons. We'll call these "failures".

Consider a program that reads in a string of input from the user and parses it
to a number. Many strings are not valid numbers, so this parsing can fail. The
only way the program could prevent that failure is by validating the string
before its parsed, but validating that a string is a number is pretty much the
same thing as parsing it.

For cases like this where failure can occur and the program *will* want to
handle it, fibers and `try()` are too coarse-grained to work with. Instead,
these operations will indicate failure by *returning* some sort of error
indication.

For example, a method for parsing a number could return a number on success and
`null` to indicate parsing failed. Since Wren is dynamically typed, it's easy
and natural for a method to return different types of values.

<br><hr>
<a class="right" href="modularity.html">Modularity &rarr;</a>
<a href="concurrency.html">&larr; Concurrency</a>
