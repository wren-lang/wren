^title Fn Class
^category core

A first class function&mdash;an object that wraps an executable chunk of code.
[Here](../functions.html) is a friendly introduction.

### new **Fn**(function)

Creates a new function from... `function`. Of course, `function` is already be
a function, so this really just returns the argument. It exists mainly to let
you create a "bare" function when you don't want to immediately pass it as a
[block argument](../functions.html#block-arguments) to some other method.

    :::dart
    var fn = new Fn {
      IO.print("The body")
    }

It is a runtime error if `block` is not a function.

### **call**(args...)

**TODO**
