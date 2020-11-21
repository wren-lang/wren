^title Functions

No self-respecting language today can get by without functions&mdash;first
class little bundles of code. Since Wren is object-oriented, most of your code
will live in methods on classes, but free-floating functions are still
eminently handy.

Functions are objects like everything else in Wren, instances of the `Fn`
class.

## Block arguments

Most of the time you create a function just to pass it to some method. For
example, if you want to filter a [list](lists.html) by some criteria, you'll
call its `where` method, passing in a function that defines the predicate
you're filtering on.

Since that's the most common usage pattern, Wren's syntax optimizes for that.
Taking a page from Ruby, a function is created by passing a *block argument* to
a method. At its simplest, it looks like this:

<pre class="snippet">
blondie.callMe {
  System.print("This is the body!")
}
</pre>

Here we're invoking the `callMe` method on `blondie`. We're passing one
argument, a function whose body is the
following [block](syntax.html#blocks)&mdash;everything between that pair of
curly braces.

Methods that take a block argument receive it as a normal parameter. `callMe`
could be defined like so:

<pre class="snippet">
class Blondie {
  callMe(fn) {
    // Call it...
  }
}

var blondie = Blondie.new()
</pre>

A method can take other arguments in addition to the block. They appear before
the block just like a regular argument list. For example:

<pre class="snippet">
blondie.callMeAt(867, 5309) {
  System.print("This is the body!")
}
</pre>

Of course, you don't *have* to use a block argument to pass a function to a
method. If you already have a function object, you can pass it like a regular
argument:

<pre class="snippet">
var someFn = // Get a function...
blondie.callMe(someFn)
</pre>

Block arguments are purely sugar for creating a function and passing it in one
little blob of syntax. There are some times when you want to create a function
but *don't* need to pass it to a method. For that, you can call the `Fn`
class's constructor:

<pre class="snippet">
var someFn = Fn.new {
  System.print("Hi!")
}
</pre>

As you can see it takes a block argument too! All the constructor does it
return that, so this exists purely as a convenience method for you.

## Calling functions

Once you have a function, how do you invoke it? Like everything in Wren, you do
so by calling a method on it:

<pre class="snippet">
class Blondie {
  callMe(fn) {
    fn.call()
  }
}
</pre>

Functions expose a `call()` method that executes the body of the function. This
method is dynamically-dispatched like any other, so you can define your own
"function-like" classes and pass them to methods that expect "real" functions.

<pre class="snippet">
class FakeFn {
  call() {
    System.print("I'm feeling functional!")
  }
}

blondie.callMe(FakeFn.new())
</pre>

## Function parameters

Of course, functions aren't very useful if you can't pass values to them. The
functions that we've seen so far take no arguments. To change that, you can
provide a parameter list surrounded by `|` immediately after the opening brace
of the body, like so:

<pre class="snippet">
blondie.callMe {|first, last|
  System.print("Hi, " + first + " " + last + "!")
}
</pre>

Here we're passing a function to `callMe` that takes two parameters, `first` and
`last`. They are passed to the function when it's called:

<pre class="snippet">
class Blondie {
  callMe(fn) {
    fn.call("Debbie", "Harry")
  }
}
</pre>

It's an error to call a function with fewer arguments than its parameter list
expects. If you pass too *many* arguments, the extras are ignored.

## Returning values

The body of a function is a [block](syntax.html#blocks). If it is a single
expression&mdash;more precisely if there is no newline after the `{` or
parameter list&mdash;then the function implicitly returns the value of the
expression.

Otherwise, the body returns `null` by default. You can explicitly return a
value using a `return` statement. In other words, these two functions do the
same thing:

<pre class="snippet">
Fn.new { "return value" }

Fn.new {
  return "return value"
}
</pre>

## Closures

As you expect, functions are closures&mdash;they can access variables defined
outside of their scope. They will hold onto closed-over variables even after
leaving the scope where the function is defined:

<pre class="snippet">
class Counter {
  static create() {
    var i = 0
    return Fn.new { i = i + 1 }
  }
}
</pre>

Here, the `create` method returns the function created on its second line. That
function references a variable `i` declared outside of the function. Even after
the function is returned from `create`, it is still able to read and assign
to`i`:

<pre class="snippet">
var counter = Counter.create()
System.print(counter.call()) //> 1
System.print(counter.call()) //> 2
System.print(counter.call()) //> 3
</pre>

<br><hr>
<a class="right" href="classes.html">Classes &rarr;</a>
<a href="variables.html">&larr; Variables</a>
