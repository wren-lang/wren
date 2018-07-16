^title Classes

Every value in Wren is an object, and every object is an instance of a class.
Even `true` and `false` are full-featured objects&mdash;instances of the
[Bool][] class.

[bool]: modules/core/bool.html

Classes define an objects *behavior* and *state*. Behavior is defined by
[*methods*][method calls] which live in the class. Every object of the same
class supports the same methods. State is defined in *fields*, whose values are
stored in each instance.

[method calls]: method-calls.html

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

This defines a `prance()` method that takes no arguments. To add parameters, put
their names inside the parentheses:

    :::wren
    class Unicorn {
      prance(where, when) {
        System.print("The unicorn prances in %(where) at %(when).")
      }
    }

Since the number of parameters is part of a method's [signature][] a class can
define multiple methods with the same name:

[signature]: method-calls.html#signature

    :::wren
    class Unicorn {
      prance() {
        System.print("The unicorn prances in a fancy manner!")
      }

      prance(where) {
        System.print("The unicorn prances in %(where).")
      }

      prance(where, when) {
        System.print("The unicorn prances in %(where) at %(when).")
      }
    }

It's often natural to have the same conceptual operation work with different
sets of arguments. In other languages, you'd define a single method for the
operation and have to check for missing optional arguments. In Wren, they are
different methods that you implement separately.

In addition to named methods with parameter lists, Wren has a bunch of other
different syntaxes for methods. Your classes can define all of them.

### Getters

A getter leaves off the parameter list and the parentheses:

    :::wren
    class Unicorn {
      // Unicorns are always fancy.
      isFancy { true }
    }

### Setters

A setter has `=` after the name, followed by a single parenthesized parameter:

    :::wren
    class Unicorn {
      rider=(value) {
        System.print("I am being ridden by %(value).")
      }
    }

By convention, the parameter is usually named `value` but you can call it
whatever makes your heart flutter.

### Operators

Prefix operators, like getters, have no parameter list:

    :::wren
    class Unicorn {
      - {
        System.print("Negating a unicorn is weird.")
      }
    }

Infix operators, like setters, have a single parenthesized parameter for the
right-hand operand:

    :::wren
    class Unicorn {
      -(other) {
        System.print("Subtracting %(other) from a unicorn is weird.")
      }
    }

A subscript operator puts the parameters inside square brackets and can have
more than one:

    :::wren
    class Unicorn {
      [index] {
        System.print("Unicorns are not lists!")
      }

      [x, y] {
        System.print("Unicorns are not matrices either!")
      }
    }

Unlike with named methods, you can't define a subscript operator with an empty
parameter list.

As the name implies, a subscript setter looks like a combination of a subscript
operator and a setter:

    :::wren
    class Unicorn {
      [index]=(value) {
        System.print("You can't stuff %(value) into me at %(index)!")
      }
    }

## Method Scope

Up to this point, "[scope][]" has been used to talk exclusively about
[variables][]. In a procedural language like C, or a functional one like Scheme,
that's the only kind of scope there is. But object-oriented languages like Wren
introduce another kind of scope: *object scope*. It contains the methods that
are available on an object. When you write:

[scope]: variables.html#scope
[variables]: variables.html

    :::wren
    unicorn.isFancy

You're saying "look up the method `isFancy` in the scope of the object
`unicorn`&rdquo;. In this case, the fact that you want to look up a *method*
`isFancy` and not a *variable* `isFancy` is explicit. That's what `.` does and
the object to the left of the period is the object you want to look up the
method on.

### `this`

Things get more interesting when you're inside the body of a method. When the
method is called on some object and the body is being executed, you often need
to access that object itself. You can do that using `this`.

    :::wren
    class Unicorn {
      name { "Francis" }

      printName() {
        System.print(this.name) //> Francis
      }
    }

The `this` keyword works sort of like a variable, but has special behavior. It
always refers to the instance whose method is currently being executed. This
lets you invoke methods on "yourself".

It's an error to refer to `this` outside of a method. However, it's perfectly
fine to use it inside a [function][] declared *inside* a method. When you do,
`this` still refers to the instance whose *method* is being called:

    :::wren
    class Unicorn {
      name { "Francis" }

      printNameThrice() {
        (1..3).each {
          // Use "this" inside the function passed to each().
          System.print(this.name) //> Francis
        } //> Francis
      } //> Francis
    }

[function]: functions.html

This is unlike Lua and JavaScript which can "forget" `this` when you create a
callback inside a method. Wren does what you want here and retains the
reference to the original object.

(In technical terms, a function's closure includes `this`. Wren can do this
because it makes a distinction between methods and functions.)

### Implicit `this`

Using `this.` every time you want to call a method on yourself works, but it's
tedious and verbose, which is why some languages don't require it. You can do a
"self send" by calling a method (or getter or setter) without any explicit
receiver:

    :::wren
    class Unicorn {
      name { "Francis" }

      printName() {
        System.print(name) //> Francis
      }
    }

Code like this gets tricky when there is also a variable outside of the class
with the same name. Consider:

    :::wren
    var name = "variable"

    class Unicorn {
      name { "Francis" }

      printName() {
        System.print(name) // ???
      }
    }

Should `printName()` print "variable" or "Francis"? A method body has a foot in
each of two worlds. It is surrounded by the lexical scope where it's defined in
the program, but it also has the object scope of the methods on `this`.

Which scope wins? Every language has to decide how to handle this and there
is a surprising plethora of approaches. Wren's approach to resolving a name
inside a method works like this:

1.  If there is a local variable inside the method with that name, that wins.
2.  Else, if the name starts with a lowercase letter, treat it like a method on
    `this`.
3.  Otherwise, look for a variable with that name in the surrounding scope.

So, in the above example, we hit case #2 and it prints "Francis". Distinguishing
self sends from outer variables based on the *case* of the first letter in the
name probably seems crazy but it works surprisingly well. Method names are
lowercase in Wren. Class names are capitalized.

Most of the time, when you're in a method and want to access a name from outside
of the class, it's usually the name of some other class. This rule makes that
work.

Here's an example that shows all three cases:

    :::wren
    var shadowed = "surrounding"
    var lowercase = "surrounding"
    var Capitalized = "surrounding"

    class Scope {
      shadowed { "object" }
      lowercase { "object" }
      Capitalized { "object" }

      test() {
        var shadowed = "local"

        System.print(shadowed) //> local
        System.print(lowercase) //> object
        System.print(Capitalized) //> surrounding
      }
    }

It's a bit of a strange rule, but Ruby works more or less the same way.

## Constructors

We've seen how to define kinds of objects and how to declare methods on them.
Our unicorns can prance around, but we don't actually *have* any unicorns to do
it. To create *instances* of a class, we need a *constructor*. You define one
like so:

    :::wren
    class Unicorn {
      construct new(name, color) {
        System.print("My name is " + name + " and I am " + color + ".")
      }
    }

The `construct` keyword says we're defining a constructor, and `new` is its
name. In Wren, all constructors have names. The word "new" isn't special to
Wren, it's just a common constructor name.

To make a unicorn now, we call the constructor method on the class itself:

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

All state stored in instances is stored in *fields*. Each field has a name
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
      construct new() {}

      static setFromStatic(a) { __a = a }
      setFromInstance(a) { __a = a }

      static printFromStatic() {
        System.print(__a)
      }

      printFromInstance() {
        System.print(__a)
      }
    }

Just like instance fields, static fields are initially `null`:

    :::wren
    Foo.printFromStatic() //> null

They can be used from static methods:

    :::wren
    Foo.setFromStatic("first")
    Foo.printFromStatic() //> first

And also instance methods. When you do so, there is still only one static field
shared among all instances of the class:

    :::wren
    var foo1 = Foo.new()
    var foo2 = Foo.new()

    foo1.setFromInstance("second")
    foo2.printFromInstance() //> second

## Inheritance

A class can inherit from a "parent" or *superclass*. When you invoke a method
on an object of some class, if it can't be found, it walks up the chain of
superclasses looking for it there.

By default, any new class inherits from Object, which is the superclass from
which all other classes ultimately descend. You can specify a different parent
class using `is` when you declare the class:

    :::wren
    class Pegasus is Unicorn {}

This declares a new class Pegasus that inherits from Unicorn.

Note that you should not create classes that inherit from the built-in types
(Bool, Num, String, Range, List). The built-in types expect their internal bit
representation to be very specific and get horribly confused when you invoke one
of the inherited built-in methods on the derived type.

The metaclass hierarchy does *not* parallel the regular class hierarchy. So, if
Pegasus inherits from Unicorn, Pegasus's metaclass does not inherit from
Unicorn's metaclass. In more prosaic terms, this means that static methods are
not inherited.

    :::wren
    class Unicorn {
      // Unicorns cannot fly. :(
      static canFly { false }
    }

    class Pegasus is Unicorn {}

    Pegasus.canFly //! Static methods are not inherited.

This also means constructors are not inherited:

    :::wren
    class Unicorn {
      construct new(name) {
        System.print("My name is " + name + ".")
      }
    }

    class Pegasus is Unicorn {}

    Pegasus.new("Fred") //! Pegasus does not define new().

Each class gets to control how it may be constructed independently of its base
classes. However, constructor *initializers* are inherited since those are
instance methods on the new object.

This means you can do `super` calls inside a constructor:

    :::wren
    class Unicorn {
      construct new(name) {
        System.print("My name is " + name + ".")
      }
    }

    class Pegasus is Unicorn {
      construct new(name) {
        super(name)
      }
    }

    Pegasus.new("Fred") //> My name is Fred

## Super

**TODO: Integrate better into page. Should explain this before mentioning
super above.**

Sometimes you want to invoke a method on yourself, but using methods defined in
one of your [superclasses](classes.html#inheritance). You typically do this in
an overridden method when you want to access the original method being
overridden.

To do that, you can use the special `super` keyword as the receiver in a method
call:

    :::wren
    class Base {
      method() {
        System.print("base method")
      }
    }

    class Derived is Base {
      method() {
        super.method() //> base method
      }
    }

You can also use `super` without a method name inside a constructor to invoke a
base class constructor:

    :::wren
    class Base {
      construct new(arg) {
        System.print("base got " + arg)
      }
    }

    class Derived is Base {
      construct new() {
        super("value") //> base got value
      }
    }

<a class="right" href="concurrency.html">Concurrency &rarr;</a>
<a href="functions.html">&larr; Functions</a>
