^title Functions

No self-respecting language today can get by without functions&mdash;first class little bundles of code. Since Wren is object-oriented, most of your code will live in methods on classes, but free-floating functions are still useful.

Functions are objects like everything else in Wren, instances of the `Fn` class.

## Creating functions

Most of the time you create a function just to pass it to some method. For example, if you want to filter a [list](lists.html) by some criteria, you'll call its `where` method, passing in a function that defines the predicate you're filtering on.

Since that's the most common usage pattern, Wren's syntax optimizes for that. Taking a page from Ruby, a function is created by passing a *block argument* to a method.

At it's simplest, it looks like this:

    :::dart
    blondie.callMe {
      IO.print("This is the body!")
    }

Here we're invoking the `callMe` method on `blondie`. We're passing one argument, a function whose body is everything between that pair of curly braces.

Methods that receive a block argument take as a normal parameter. `callMe` could be defined like so:

    :::dart
    class Blondie {
      callMe(fn) {
        // Call it...
      }
    }

A method can take other arguments in addition to the block. They appear before the block just like a regular argument list. For example:

    :::dart
    blondie.callMeAt(867, 5309) {
      IO.print("This is the body!")
    }

Of course, you don't *have* to use a block argument to pass a function to a method. If you already have a function object, you can pass it like a regular argument:

    :::dart
    var someFn = // Get a function...
    blondie.callMe(someFn)

Block arguments are purely sugar for creating a function and passing it in one little blob of syntax. There are some times when you want to create a function but *don't* need to pass it to a method. For that, you can call the `Fn` class's constructor:

    var someFn = new Fn {
      IO.print("Hi!")
    }

As you can see it takes a block argument too! All the constructor does it return that, so there's no special syntax here.

## Calling functions

Once you have a function, how do you invoke it? Like everything in Wren, you do so by calling a method on it:

    :::dart
    class Blondie {
      callMe(fn) {
        fn.call
      }
    }

Functions expose a `call` method that executes the body of the function. Of course, this is dynamically-dispatched like other methods, so you can define your own "function-like" classes and pass them to methods that expect real functions.

    :::dart
    class FakeFn {
      call {
        IO.print("I'm feeling functional!")
      }
    }

    blondie.callMe(new FakeFn)

## Function parameters

**TODO**

**TODO: Implicit returns from short bodies.**

## Closures

As you expect, functions are closures: they can access variables defined outside of their scope. They will hold onto closed-over variables even after leaving the scope where the function is defined:

    :::dart
    class Counter {
      static create {
        var i = 0
        return new Fn { i = i + 1 }
      }
    }

Here, the `create` method returns the function created on its second line. That function references a variable `i` declared outside of the function. Even after the function is returned from `create`, it is still able to access `i`.

    :::dart
    var counter = Counter.create
    IO.print(counter.call) // Prints "1".
    IO.print(counter.call) // Prints "2".
    IO.print(counter.call) // Prints "3".
