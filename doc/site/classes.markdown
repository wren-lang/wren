^title Classes

Every value in Wren is an object, and every object is an instance of a class.
Even `true` and `false` are full-featured objects, instances of the `Bool` class.

## Defining a class

Classes are created using the `class` keyword, unsurprisingly:

    :::wren
    class Unicorn {}

This creates a class named `Unicorn` with no methods or fields.

**TODO: methods**

## Inheritance

A class can inherit from a "parent" or *superclass*. When you invoke a method on an object of some class, if it can't be found, it walks up the chain of superclasses looking for it there.

By default, any new class inherits from `Object`, which is the superclass from which all other classes ultimately descend. You can specify a different parent class using `is` when you declare the class:

    :::class
    class Pegasus is Unicorn {}

This declares a new class `Pegasus` that inherits from `Unicorn`.

**TODO metaclasses, supercalls, fields, etc.**
