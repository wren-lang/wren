^title Fiber Class
^category core

A lightweight coroutine. [Here](../fibers.html) is a gentle introduction.

### new **Fiber**(function)

Creates a new fiber that executes `function` in a separate coroutine when the
fiber is run. Does not immediately start running the fiber.

    :::dart
    var fiber = new Fiber {
      IO.print("I won't get printed")
    }

### Fiber.**yield**

Pauses the current fiber and transfers control to the parent fiber. "Parent"
here means the last fiber that was started using `call` and not `run`.

    :::dart
    var fiber = new Fiber {
      IO.print("Before yield")
      Fiber.yield
      IO.print("After yield")
    }

    fiber.call              // "Before yield"
    IO.print("After call")  // "After call"
    fiber.call              // "After yield"

When resumed, the parent fiber's `call` method returns `null`.

If a yielded fiber is resumed by calling `call()` or `run()` with an argument,
`yield` returns that value.

    :::dart
    var fiber = new Fiber {
      IO.print(Fiber.yield) // "value"
    }

    fiber.call          // Run until the first yield.
    fiber.call("value") // Resume the fiber.

If it was resumed by calling `call` or `run` with no argument, returns `null`.

It is a runtime error to call this when there is no parent fiber to return to.

    :::dart
    Fiber.yield // ERROR

    new Fiber {
      Fiber.yield // ERROR
    }.run

### Fiber.**yield**(value)

Similar to `Fiber.yield` but provides a value to return to the parent fiber's
`call`.

    :::dart
    var fiber = new Fiber {
      Fiber.yield("value")
    }

    IO.print(fiber.call) // "value"

### **call**

**TODO**

### **call**(value)

**TODO**

### **isDone**

Whether the fiber's main function has completed and the fiber can no longer be
run. This returns `false` if the fiber is currently running or has yielded.

### **run**

**TODO**

### **run**(value)

**TODO**
