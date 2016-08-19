^title Fn Class

A first class function&mdash;an object that wraps an executable chunk of code.
[Here][functions] is a friendly introduction.

[functions]: ../../functions.html

## Static Methods

### Fn.**new**(function)

Creates a new function from... `function`. Of course, `function` is already a
function, so this really just returns the argument. It exists mainly to let you
create a "bare" function when you don't want to immediately pass it as a [block
argument](../functions.html#block-arguments) to some other method.

    :::wren
    var fn = Fn.new {
      System.print("The body")
    }

It is a runtime error if `function` is not a function.

## Methods

### **arity**

The number of arguments the function requires.

    :::wren
    System.print(Fn.new {}.arity)             //> 0
    System.print(Fn.new {|a, b, c| a }.arity) //> 3

### **call**(args...)

Invokes the function with the given arguments.

    :::wren
    var fn = Fn.new { |arg|
      System.print(arg)     //> Hello world
    }

    fn.call("Hello world")

It is a runtime error if the number of arguments given is less than the arity
of the function. If more arguments are given than the function's arity they are
ignored.
