^title Classes

Every value in Wren is an object, and every object is an instance of a class.
Even `true` and `false` are full-featured objects, instances of the `Bool` class.

Classes contain both *behavior* and *state*. Behavior is defined in *methods* which are stored in the class. State is defined in *fields*, whose values are stored in each instance.

## Defining a class

Classes are created using the `class` keyword, unsurprisingly:

    :::wren
    class Unicorn {}

This creates a class named `Unicorn` with no methods or fields.

## Defining methods

To make our unicorn do stuff, we need to give it methods.

    :::wren
    class Unicorn {
      prance {
        IO.write("The unicorn prances in a fancy manner!")
      }
    }

This defines a `prance` method that takes no arguments. To support parameters, add a parenthesized parameter list after the method's name:

    :::wren
    // Inside class...
    prance(where, when) {
      IO.write("The unicorn prances in " + where + " at " + when)
    }

Unlike most other dynamically-typed languages, in Wren you can have multiple methods in a class with the same name, as long as they take a different number of parameters. In other words, you can overload by arity. So this class is fine:

    :::wren
    class Unicorn {
      prance {
        IO.write("The unicorn prances in a fancy manner!")
      }

      prance(where) {
        IO.write("The unicorn prances in " + where)
      }

      prance(where, when) {
        IO.write("The unicorn prances in " + where + " at " + when)
      }
    }

When you [call](method-calls.html) the `prance` method, it will select the right one based on how many arguments you pass it.

**TODO: Defining operators and setters.**

## Constructors

**TODO**

## Fields

**TODO**

## Metaclasses and static members

**TODO**

## Inheritance

A class can inherit from a "parent" or *superclass*. When you invoke a method on an object of some class, if it can't be found, it walks up the chain of superclasses looking for it there.

By default, any new class inherits from `Object`, which is the superclass from which all other classes ultimately descend. You can specify a different parent class using `is` when you declare the class:

    :::class
    class Pegasus is Unicorn {}

This declares a new class `Pegasus` that inherits from `Unicorn`.

The metaclass hierarchy parallels the regular class hierarchy. So, if `Pegasus` inherits from `Unicorn`, `Pegasus`'s metaclass will inherit from `Unicorn`'s metaclass. In more prosaic terms, this means that static methods are inherited and can be overridden. This includes constructors.

If you *don't* want your subclass to support some static method that it's superclass has, the simplest solution is to override it and make your method error out.

## Superclass method calls

**TODO**
