^title Fn Class
^category core

A first class function&mdash;an object that wraps an executable chunk of code.
[Here](../functions.html) is a friendly introduction.

### new **Fn**(function)

Creates a new function from... `function`. Of course, `function` is already a
function, so this really just returns the argument. It exists mainly to let you
create a "bare" function when you don't want to immediately pass it as a [block
argument](../functions.html#block-arguments) to some other method.

    :::dart
    var fn = new Fn {
      IO.print("The body")
    }

It is a runtime error if `function` is not a function.

## Methods

### **arity**

The number of arguments the function requires.

    :::dart
    IO.print(new Fn {}.arity)             // 0.
    IO.print(new Fn {|a, b, c| a }.arity) // 3.

### **call**(args...)

Invokes the function with the given arguments.

    :::dart
    var fn = new Fn { |arg|
      IO.print(arg)
    }
    fn.call("Hello world") // Hello world.

It is a runtime error if the number of arguments given does not equal the arity
of the function.