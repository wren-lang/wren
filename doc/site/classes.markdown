^title Classes

Every value in Wren is an object, and every object is an instance of a class.
Even `true` and `false` are full-featured objects&mdash;instances of the `Bool` class.

Classes contain both *behavior* and *state*. Behavior is defined in *methods* which are stored in the class. State is defined in *fields*, whose values are stored in each instance.

## Defining a class

Classes are created using the `class` keyword, unsurprisingly:

    :::dart
    class Unicorn {}

This creates a class named `Unicorn` with no methods or fields.

## Methods

Once we've made a unicorn, to let it do stuff, we need to give it methods.

    :::dart
    class Unicorn {
      prance {
        IO.print("The unicorn prances in a fancy manner!")
      }
    }

This defines a `prance` method that takes no arguments. To support parameters, add a parenthesized parameter list after the method's name:

    :::dart
    class Unicorn {
      prance(where, when) {
        IO.print("The unicorn prances in " + where + " at " + when)
      }
    }

Unlike most other dynamically-typed languages, in Wren you can have multiple methods in a class with the same name, as long as they take a different number of parameters. In technical terms, you can overload by *arity*. So this class is fine:

    :::dart
    class Unicorn {
      prance {
        IO.print("The unicorn prances in a fancy manner!")
      }

      prance(where) {
        IO.print("The unicorn prances in " + where)
      }

      prance(where, when) {
        IO.print("The unicorn prances in " + where + " at " + when)
      }
    }

When you [call](method-calls.html) the `prance` method, it selects the right one based on how many arguments you pass it.

## Constructors

To create a new instance of a class, you use the `new` keyword. We can make a unicorn like so:

    :::dart
    new Unicorn

You almost always want to define some state or do some other initialization on a new object. For that, you'll want to define a constructor, like so:

    :::dart
    class Unicorn {
      new {
        IO.print("I am a constructor!")
      }
    }

When you create an instance with `new`, its constructor will be invoked. It's just a method with a special name. Like methods, you can pass arguments to the constructor by adding a parenthesized parameter list after `new`:

    :::dart
    class Unicorn {
      new(name, color) {
        IO.print("My name is " + name + " and I am " + color + ".")
      }
    }

Values are passed to the constructor like so:

    :::dart
    new Unicorn("Flicker", "purple")

Like other methods, you can overload constructors by arity.

## Operators

Operators are just special syntax for regular [method calls](method-calls.html) on the left hand operand (or only operand in the case of unary operators like `!` and `~`). You can define them like so:

    :::dart
    class Unicorn {
      // Infix:
      +(other) {
        IO.print("Adding to a unicorn?")
      }

      // Prefix:
      ! {
        IO.print("Negating a unicorn?!")
      }
    }

This can be used to define any of these operators:

    :::dart
    // Infix:
    +  -  *  /  %  <  >  <=  >=  ==  !=  &  |

    // Prefix:
    !  ~  -

Note that `-` can be both a prefix and infix operator. If there's a parameter list, it's the infix one, otherwise, it's prefix. Since Wren supports overloading by arity, it's no problem for a class to define both.

Operator overloading is really useful for types like vectors and complex numbers where the reader knows what the operators will do, but can make code deeply confusing if overused. When in doubt, use a real name.

## Setters

[Assignment](variables.html) *cannot* be overloaded. It isn't an operator, and its semantics are built right into the language.

**TODO: ...**

## Fields

**TODO**

## Metaclasses and static members

**TODO**

## Inheritance

A class can inherit from a "parent" or *superclass*. When you invoke a method on an object of some class, if it can't be found, it walks up the chain of superclasses looking for it there.

By default, any new class inherits from `Object`, which is the superclass from which all other classes ultimately descend. You can specify a different parent class using `is` when you declare the class:

    :::dart
    class Pegasus is Unicorn {}

This declares a new class `Pegasus` that inherits from `Unicorn`.

The metaclass hierarchy does *not* parallel the regular class hierarchy. So, if `Pegasus` inherits from `Unicorn`, `Pegasus`'s metaclass will not inherit from `Unicorn`'s metaclass. In more prosaic terms, this means that static methods are not inherited.

Constructors, however, initialize the instance *after* it has been created. They are defined as instance methods on the class and not on the metaclass. That means that constructors *are* inherited.

## Superclass method calls

**TODO**
