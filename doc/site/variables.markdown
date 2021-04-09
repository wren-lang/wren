^title Variables

Variables are named slots for storing values. You define a new variable in Wren
using a `var` statement, like so:

<pre class="snippet">
var a = 1 + 2
</pre>

This creates a new variable `a` in the current scope and initializes it with
the result of the expression following the `=`. Once a variable has been
defined, it can be accessed by name as you would expect.

<pre class="snippet">
var animal = "Slow Loris"
System.print(animal) //> Slow Loris
</pre>

## Scope

Wren has true block scope: a variable exists from the point where it is defined
until the end of the [block](syntax.html#blocks) where that definition appears.

<pre class="snippet">
{
  System.print(a) //! "a" doesn't exist yet.
  var a = 123
  System.print(a) //> 123
}
System.print(a) //! "a" doesn't exist anymore.
</pre>

Variables defined at the top level of a script are *top-level* and are visible
to the [module](modularity.html) system. All other variables are *local*.
Declaring a variable in an inner scope with the same name as an outer one is
called *shadowing* and is not an error (although it's not something you likely
intend to do much).

<pre class="snippet">
var a = "outer"
{
  var a = "inner"
  System.print(a) //> inner
}
System.print(a) //> outer
</pre>

Declaring a variable with the same name in the *same* scope *is* an error.

<pre class="snippet">
var a = "hi"
var a = "again" //! "a" is already declared.
</pre>

## Assignment

After a variable has been declared, you can assign to it using `=`

<pre class="snippet">
var a = 123
a = 234
</pre>

An assignment walks up the scope stack to find where the named variable is
declared. It's an error to assign to a variable that isn't defined. Wren
doesn't roll with implicit variable definition.

When used in a larger expression, an assignment expression evaluates to the
assigned value.

<pre class="snippet">
var a = "before"
System.print(a = "after") //> after
</pre>

If the left-hand side is some more complex expression than a bare variable name,
then it isn't an assignment. Instead, it's calling a [setter method][].

[setter method]: method-calls.html#setters

<br><hr>
<a class="right" href="functions.html">Functions &rarr;</a>
<a href="control-flow.html">&larr; Control Flow</a>
