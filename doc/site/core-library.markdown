^title Core Library
^category reference

## Object Class

### **==**(other) and **!=**(other) operators

Compares two objects using built-in equality. This compares numbers by value,
and all other objects are compared by identity&mdash;two objects are equal only
if they are the exact same object.

### **toString**

A default string representation of the object.

### **type**

The [Class](#class-class) of the object.

## Class Class

### **name**

The name of the class.

## Bool Class

Boolean values. There are two instances, `true` and `false`.

### **!** operator

Returns the logical complement of the value.

    > !true
    false
    > !false
    true

### toString

The string representation of the value, either `"true"` or `"false"`.

## Fiber Class

A lightweight coroutine. [Here](fibers.html) is a gentle introduction.

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

## Fn Class

A first class function&mdash;an object that wraps an executable chunk of code.
[Here](functions.html) is a friendly introduction.

### new **Fn**(function)

Creates a new function from... `function`. Of course, `function` is already be
a function, so this really just returns the argument. It exists mainly to let
you create a "bare" function when you don't want to immediately pass it as a
[block argument](functions.html#block-arguments) to some other method.

    :::dart
    var fn = new Fn {
      IO.print("The body")
    }

It is a runtime error if `block` is not a function.

### **call**(args...)

**TODO**

## Null Class

### **!** operator

Returns `true`, since `null` is considered [false](control-flow.html#truth).

    > !null
    true

## Num Class

**TODO**

### **abs**

The absolute value of the number.

    :::dart
    -123.abs // 123

### **ceil**

**TODO**

### **cos**

The cosine of the number.

### **floor**

**TODO**

### **isNan**

Whether the number is [not a number](http://en.wikipedia.org/wiki/NaN). This is
`false` for normal number values and infinities, and `true` for the result of
`0/0`, the square root of a negative number, etc.

### **sin**

The sine of the number.

### **sqrt**

The square root of the number. Returns `nan` if the number is negative.

### **-** operator

Negates the number.

    :::dart
    var a = 123
    -a // -123

### **-**(other), **+**(other), **/**(other), **\***(other) operators

The usual arithmetic operators you know and love. All of them do 64-bit
floating point arithmetic. It is a runtime error if the right-hand operand is
not a number. Wren doesn't roll with implicit conversions.

### **%**(denominator) operator

The floating-point remainder of this number divided by `denominator`.

It is a runtime error if `denominator` is not a number.

### **&lt;**(other), **&gt;**(other), **&lt;=**(other), **&gt;=**(other) operators

Compares this and `other`, returning `true` or `false` based on how the numbers
are ordered. It is a runtime error if `other` is not a number.

### **~** operator

Performs *bitwise* negation on the number. The number is first converted to a
32-bit unsigned value, which will truncate any floating point value. The bits
of the result of that are then negated, yielding the result.

### **&**(other) operator

Performs bitwise and on the number. Both numbers are first converted to 32-bit
unsigned values. The result is then a 32-bit unsigned number where each bit is
`true` only where the corresponding bits of both inputs were `true`.

It is a runtime error if `other` is not a number.

### **|**(other) operator

Performs bitwise or on the number. Both numbers are first converted to 32-bit
unsigned values. The result is then a 32-bit unsigned number where each bit is
`true` only where the corresponding bits of both inputs were `true`.

It is a runtime error if `other` is not a number.

### **..**(other) operator

**TODO**

### **...**(other) operator

**TODO**

## Object Class

### **!** operator

Returns `false`, since most objects are considered [true](control-flow.html#truth).

## String Class

A string of Unicode code points stored in UTF-8.

### **contains**(other)

Checks if `other` is a substring of the string.

It is a runtime error if `other` is not a string.

### **count**

Returns the length of the string.

### **endsWith(suffix)**

Checks if the string ends with `suffix`.

It is a runtime error if `suffix` is not a string.

### **indexOf(search)**

Returns the index of `search` in the string or -1 if `search` is not a
substring of the string.

It is a runtime error if `search` is not a string.

### **startsWith(prefix)**

Checks if the string starts with `prefix`.

It is a runtime error if `prefix` is not a string.

### **+**(other) operator

Returns a new string that concatenates this string and `other`.

It is a runtime error if `other` is not a string.

### **==**(other) operator

Checks if the string is equal to `other`.

### **!=**(other) operator

Check if the string is not equal to `other`.

### **[**index**]** operator

Returns a one character string of the value at `index`.

It is a runtime error if `index` is greater than the length of the string.

*Note: This does not currently handle UTF-8 characters correctly.*

## List Class

**TODO**

### **add**(item)

Appends `item` onto the end of the list.

### **clear**

Removes all items from the list.

### **count**

The number of items in the list.

### **all(predicate)**

Tests whether all the elements in the list pass the `predicate`.

### **insert**(item, index)

**TODO**

### **iterate**(iterator), **iteratorValue**(iterator)

**TODO**

### **removeAt**(index)

**TODO**

### **[**index**]** operator

**TODO**

### **[**index**]=**(item) operator

**TODO**

## Range Class

**TODO**

### **from**

**TODO**

### **to**

**TODO**

### **min**

**TODO**

### **max**

**TODO**

### **isInclusive**

**TODO**

### **iterate**(iterator), **iteratorValue**(iterator)

**TODO**
