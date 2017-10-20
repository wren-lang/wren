^title Fiber Class

A lightweight coroutine. [Here][fibers] is a gentle introduction.

[fibers]: ../../concurrency.html

## Static Methods

### Fiber.**abort**(message)

Raises a runtime error with the provided message:

    :::wren
    Fiber.abort("Something bad happened.")

If the message is `null`, does nothing.

### Fiber.**current**

The currently executing fiber.

### Fiber.**new**(function)

Creates a new fiber that executes `function` in a separate coroutine when the
fiber is run. Does not immediately start running the fiber.

    :::wren
    var fiber = Fiber.new {
      System.print("I won't get printed")
    }

`function` must be a function (an actual [Fn][] instance, not just an object
with a `call()` method) and it may only take zero or one parameters.

[fn]: fn.html

### Fiber.**suspend**()

Pauses the current fiber, and stops the interpreter. Control returns to the
host application.

Typically, you store a reference to the fiber using `Fiber.current` before
calling this. The fiber can be resumed later by calling or transferring to that
reference. If there are no references to it, it is eventually garbage collected.

Much like `yield()`, returns the value passed to `call()` or `transfer()` when
the fiber is resumed.

### Fiber.**yield**()

Pauses the current fiber and transfers control to the parent fiber. "Parent"
here means the last fiber that was started using `call` and not `transfer`.

    :::wren
    var fiber = Fiber.new {
      System.print("Before yield")
      Fiber.yield()
      System.print("After yield")
    }

    fiber.call()                //> Before yield
    System.print("After call")  //> After call
    fiber.call()                //> After yield

When resumed, the parent fiber's `call()` method returns `null`.

If a yielded fiber is resumed by calling `call()` or `transfer()` with an
argument, `yield()` returns that value.

    :::wren
    var fiber = Fiber.new {
      System.print(Fiber.yield()) //> value
    }

    fiber.call()        // Run until the first yield.
    fiber.call("value") // Resume the fiber.

If it was resumed by calling `call()` or `transfer()` with no argument, it
returns `null`.

If there is no parent fiber to return to, this exits the interpreter. This can
be useful to pause execution until the host application wants to resume it
later.

    :::wren
    Fiber.yield()
    System.print("this does not get reached")

### Fiber.**yield**(value)

Similar to `Fiber.yield` but provides a value to return to the parent fiber's
`call`.

    :::wren
    var fiber = Fiber.new {
      Fiber.yield("value")
    }

    System.print(fiber.call()) //> value

## Methods

### **call**()

Starts or resumes the fiber if it is in a paused state. Equivalent to:

    :::wren
    fiber.call(null)

### **call**(value)

Start or resumes the fiber if it is in a paused state. If the fiber is being
started for the first time, and its function takes a parameter, `value` is
passed to it.

    :::wren
    var fiber = Fiber.new {|param|
      System.print(param) //> begin
    }

    fiber.call("begin")

If the fiber is being resumed, `value` becomes the returned value of the fiber's
call to `yield`.

    :::wren
    var fiber = Fiber.new {
      System.print(Fiber.yield()) //> resume
    }

    fiber.call()
    fiber.call("resume")

### **error**

The error message that was passed when aborting the fiber, or `null` if the
fiber has not been aborted.

    :::wren
    var fiber = Fiber.new {
      123.badMethod
    }

    fiber.try()
    System.print(fiber.error) //> Num does not implement method 'badMethod'.

### **isDone**

Whether the fiber's main function has completed and the fiber can no longer be
run. This returns `false` if the fiber is currently running or has yielded.

### **try**()
Tries to run the fiber. If a runtime error occurs
in the called fiber, the error is captured and is returned as a string.

    :::wren
    var fiber = Fiber.new {
      123.badMethod
    }

    var error = fiber.try()
    System.print("Caught error: " + error)

If the called fiber raises an error, it can no longer be used.

### **transfer**()

**TODO**

### **transfer**(value)

**TODO**

### **transferError**(error)

**TODO**
