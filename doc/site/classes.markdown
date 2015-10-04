^title Classes
^category types

Every value in Wren is an object, and every object is an instance of a class.
Even `true` and `false` are full-featured objects&mdash;instances of the
[`Bool`](core/bool.html) class.

Classes contain both *behavior* and *state*. Behavior is defined in *methods*
which are stored in the class. State is defined in *fields*, whose values are
stored in each instance.

## Defining a class

Classes are created using the `class` keyword, unsurprisingly:

    :::wren
    class Unicorn {}

This creates a class named `Unicorn` with no methods or fields.

## Methods

To let our unicorn do stuff, we need to give it methods.

    :::wren
    class Unicorn {
      prance() {
        System.print("The unicorn prances in a fancy manner!")
      }
    }

This defines a `prance()` method that takes no arguments. To support
parameters, put their names inside the parentheses:

    :::wren
    class Unicorn {
      prance(where, when) {
        System.print("The unicorn prances in " + where + " at " + when)
      }
    }

### Signature

Unlike most other dynamically-typed languages, in Wren you can have multiple
methods in a class with the same name, as long as they have a different
*signature*. In technical terms, you can *overload by arity*. So this class is
fine:

    :::wren
    class Unicorn {
      prance() {
        System.print("The unicorn prances in a fancy manner!")
      }

      prance(where) {
        System.print("The unicorn prances in " + where)
      }

      prance(where, when) {
        System.print("The unicorn prances in " + where + " at " + when)
      }
    }

And you can call each of the methods like so:

    :::wren
    var unicorn = Unicorn.new()
    unicorn.prance()
    unicorn.prance("Antwerp")
    unicorn.prance("Brussels", "high noon")

The number of arguments provided at the callsite determines which method is
chosen.

It's often natural to have the same conceptual operation work with different
sets of arguments. In other languages, you'd define a single method for the
operation and have to check for "undefined" or missing arguments. Wren just
treats them as different methods that you can implement separately.

### Getters

Many methods on a class exist to expose or compute some property of the object.
For example:

    :::wren
    System.print("string".count) // "6".

These *getters* are just another kind of method&mdash;one without a parameter
list. You can define them like so:

    :::wren
    class Unicorn {
      isFancy { true } // Unicorns are always fancy.
    }

Whether or not a method name has parentheses is also part of its signature.
This lets you distinguish between a method that takes an *empty* argument list
(`()`) and no argument list at all:

    :::wren
    class Confusing {
      method { "no argument list" }
      method() { "empty argument list" }
    }

    var confusing = Confusing.new()
    confusing.method // "no argument list".
    confusing.method() // "empty argument list".

Like the example says, having two methods that differ just by an empty set of
parentheses is pretty confusing. That's not what this is for. Instead, it
ensures that the way you *declare* the method is the way you *call* it.

Unlike other languages with "optional parentheses", Wren wants to make sure you
call a getter like a getter and a `()` method like a `()` method. These don't
work:

    :::wren
    "string".count()
    list.clear

Methods that don't need arguments and don't modify the underlying object are
usually getters:

    :::wren
    "string".count
    (1..10).min
    1.23.sin
    [1, 2, 3].isEmpty

When a method doesn't need any parameters, but does modify the object, it's
helpful to draw attention to that by requiring an empty set of parentheses:

    :::wren
    list.clear()

Also, when a method supports multiple arities, it's typical to include the `()`
in the zero-argument case to be consistent with the other versions:

    Fn.new { "a function" }.call()
    Fiber.yield()

### Operators

Operators are just special syntax for a method call on the left hand operand
(or only operand in the case of unary operators like `!` and `~`). In other
words, you can think of `a + b` as meaning `a.+(b)`.

You can define operators in your class like so:

    :::wren
    class Unicorn {
      // Infix:
      +(other) {
        System.print("Adding to a unicorn?")
      }

      // Prefix:
      ! {
        System.print("Negating a unicorn?!")
      }
    }

This can be used to define any of these operators:

    :::wren
    // Infix:
    +  -  *  /  %  <  >  <=  >=  ==  !=  &  |

    // Prefix:
    !  ~  -

Note that `-` can be both a prefix and infix operator. If there's a parameter
list, it's the infix one, otherwise, it's prefix. Since Wren supports
overloading by arity, it's no problem for a class to define both.

### Subscript operators

**TODO**

### Setters

**TODO**

## Constructors

Unicorns can prance around now, but we don't actually *have* any unicorns to do
it. To create instances of a class, we need a *constructor*. You can define one
like so:

    :::wren
    class Unicorn {
      construct new(name, color) {
        System.print("My name is " + name + " and I am " + color + ".")
      }
    }

The `construct` keyword says we're defining a constructor, and `new` is its
name. In Wren, all constructors have names, just like [methods][#methods]. The
word "new" isn't special to Wren, it's just a common constructor name.

To make a unicorn now, we just call the constructor method on the class itself:

    :::wren
    var fred = Unicorn.new("Fred", "palomino")

Giving constructors names is handy because it means you can have more than one,
and each can clarify how it creates the instance:

    :::wren
    class Unicorn {
      construct brown(name) {
        System.print("My name is " + name + " and I am brown.")
      }
    }

    var dave = Unicorn.brown("Dave")

Note that we have to declare a constructor because, unlike some other
languages, Wren doesn't give you a default one. This is useful because some
classes aren't designed to be constructed. If you have an abstract base class
that just contains methods to be inherited by other classes, it doesn't need
and won't have a constructor.

Like other methods, constructors can obviously have arguments, and can be
overloaded by [arity](#signature). A constructor *must* be a named method with
a (possibly empty) argument list. Operators, getters, and setters cannot be
constructors.

A constructor is actually a pair of methods. You get a method on the class:

    :::wren
    Unicorn.brown("Dave")

That creates the new instance, then it invokes the *initializer* on that
instance. This is where the constructor body you defined gets run.

This distinction is important because it means inside the body of the
constructor, you can access `this`, assign [fields](#fields), call superclass
constructors, etc.

## Fields

All state stored in instances is stored in *fields*. Each field has a named
that starts with an underscore.

    :::wren
    class Rectangle {
      area { _width * _height }

      // Other stuff...
    }

Here, `_width` and `_height` in the `area` [getter](classes.html#methods) refer
to fields on the rectangle instance. You can think of them like `this.width`
and `this.height` in other languages.

When a field name appears, Wren looks for the nearest enclosing class and looks
up the field on the instance of that class. Field names cannot be used outside
of an instance method. They *can* be used inside a [function](functions.html)
in a method. Wren will look outside any nested functions until it finds an
enclosing method.

Unlike [variables](variables.html), fields are implicitly declared by simply
assigning to them. If you access a field before it has been initialized, its
value is `null`.

### Encapsulation

All fields are *private* in Wren&mdash;an object's fields can only be directly
accessed from within methods defined on the object's class. You cannot even
access fields on another instance of your own class, unlike C++ and Java.

If you want to make a property of an object visible, you need to define a
getter to expose it:

    :::wren
    class Rectangle {
      width { _width }
      height { _height }

      // ...
    }

To allow outside code to modify the field, you'll also need to provide setters:

    :::wren
    class Rectangle {
      width=(value) { _width = value }
      height=(value) { _height = value }
    }

One thing we've learned in the past forty years of software engineering is that
encapsulating state tends to make code easier to maintain, so Wren defaults to
keeping your object's state pretty tightly bundled up. Don't feel that you have
to or even should define getters or setters for most of your object's fields.

## Metaclasses and static members

**TODO**

### Static fields

A name that starts with *two* underscores is a *static* field. They work
similar to [fields](#fields) except the data is stored on the class itself, and
not the instance. They can be used in *both* instance and static methods.

    :::wren
    class Foo {
      // Set the static field.
      static set(a) {
        __a = a
      }

      setFromInstance(a) {
        __a = a
      }

      // Can use __a in both static methods...
      static bar { __a }

      // ...and instance ones.
      baz { __a }
    }

Just like instance fields, static fields are initially `null`:

    :::wren
    System.print(Foo.bar) // null.

They can be used from static methods:

    :::wren
    Foo.set("foo")
    System.print(Foo.bar) // foo.

And also instance methods. When you do so, there is still only one static field
shared among all instances of the class:

    :::wren
    var foo1 = Foo.new()
    var foo2 = Foo.new()

    foo1.setFromInstance("updated")
    System.print(foo2.baz) // updated.

## Inheritance

A class can inherit from a "parent" or *superclass*. When you invoke a method
on an object of some class, if it can't be found, it walks up the chain of
superclasses looking for it there.

By default, any new class inherits from `Object`, which is the superclass from
which all other classes ultimately descend. You can specify a different parent
class using `is` when you declare the class:

    :::wren
    class Pegasus is Unicorn {}

This declares a new class `Pegasus` that inherits from `Unicorn`.

Note that you should not create classes that inherit from the built-in types (Bool, Num, String, Range, List). The built-in types expect their internal bit representation to be very specific and get horribly confused when you invoke one of the inherited built-in methods on the derived type.

The metaclass hierarchy does *not* parallel the regular class hierarchy. So, if
`Pegasus` inherits from `Unicorn`, `Pegasus`'s metaclass will not inherit from
`Unicorn`'s metaclass. In more prosaic terms, this means that static methods
are not inherited.

    :::wren
    class Unicorn {
      // Unicorns cannot fly. :(
      static canFly { false }
    }

    class Pegasus is Unicorn {}

    Pegasus.canFly // ERROR: Static methods are not inherited.

This also means constructors are not inherited:

    :::wren
    class Unicorn {
      this new(name) {
        System.print("My name is " + name + ".")
      }
    }

    class Pegasus is Unicorn {}

    Pegasus.new("Fred") // Error!

Each class gets to control how it may be constructed independently of its base
classes. However, constructor *initializers* are inherited since those are
instance methods on the new object.

This means you can do `super` calls inside a constructor:

    :::wren
    class Unicorn {
      this new(name) {
        System.print("My name is " + name + ".")
      }
    }

    class Pegasus is Unicorn {
      this new(name) {
        super(name)
      }
    }

    Pegasus.new("Fred") // Prints "My name is Fred.".
