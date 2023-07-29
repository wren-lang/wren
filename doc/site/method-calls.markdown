^title Method Calls

Wren is deeply object oriented, so most code consists of invoking methods on
objects, usually something like this:

<pre class="snippet">
System.print("Heyoo!") //> Heyoo!
</pre>

You have a *receiver* expression (here `System`) followed by a `.`, then a name
(`print`) and an argument list in parentheses (`("Heyoo!")`). Multiple arguments
are separated by commas:

<pre class="snippet">
list.insert(3, "item")
</pre>

The argument list can also be empty:

<pre class="snippet">
list.clear()
</pre>

The VM executes a method call like so:

1. Evaluate the receiver and arguments from left to right.
2. Look up the method on the receiver's [class][].
3. Invoke it, passing in the argument values.

[class]: classes.html

## Signature

Unlike most other dynamically-typed languages, in Wren a class can have multiple
methods with the same *name*, as long as they have different *signatures*. The
signature includes the method's name along with the number of arguments it
takes. In technical terms, this means you can *overload by arity*.

For example, the [Random][] class has two methods for getting a random integer.
One takes a minimum and maximum value and returns a value in that range. The
other only takes a maximum value and uses 0 as the minimum:

[random]: modules/random/random.html

<pre class="snippet">
var random = Random.new()
random.int(3, 10)
random.int(4)
</pre>

In a language like Python or JavaScript, these would both call a single `int()`
method, which has some kind of "optional" parameter. The body of the method
figures out how many arguments were passed and uses control flow to handle the
two different behaviors. That means first parameter represents "max unless
another parameter was passed, in which case it's min". 

This type of 'variadic' code isn't ideal, so Wren doesn't encourage it.

In Wren, these are calls to two entirely separate methods, `int(_,_)` and
`int(_)`. This makes it easier to define "overloads" like this since you don't
need optional parameters or any kind of control flow to handle the different
cases.

It's also faster to execute. Since we know how many arguments are passed at
compile time, we can compile this to directly call the right method and avoid
any "if I got two arguments do this..." runtime work.

## Getters

Some methods exist to expose a stored or computed property of an object. These
are *getters* and have no parentheses:

<pre class="snippet">
"string".count    //> 6
(1..10).min       //> 1
1.23.sin          //> 0.9424888019317
[1, 2, 3].isEmpty //> false
</pre>

A getter is *not* the same as a method with an empty argument list. The `()` is
part of the signature, so `count` and `count()` have different signatures.
Unlike Ruby's optional parentheses, Wren wants to make sure you call a getter
like a getter and a `()` method like a `()` method. These don't work:

<pre class="snippet">
"string".count()
[1, 2, 3].clear
</pre>

If you're defining some member that doesn't need any parameters, you need to
decide if it should be a getter or a method with an empty `()` parameter list.
The general guidelines are:

*   If it modifies the object or has some other side effect, make it a method:

<pre class="snippet">
list.clear()
</pre>

*   If the method supports multiple arities, make the zero-parameter case a `()`
    method to be consistent with the other versions:

<pre class="snippet">
Fiber.yield()
Fiber.yield("value")
</pre>

*   Otherwise, it can probably be a getter.

## Setters

A getter lets an object expose a public "property" that you can *read*.
Likewise, a *setter* lets you write to a property:

<pre class="snippet">
person.height = 74 // Grew up!
</pre>

Despite the `=`, this is just another syntax for a method call. From the
language's perspective, the above line is just a call to the `height=(_)`
method on `person`, passing in `74`.

Since the `=(_)` is in the setter's signature, an object can have both a getter
and setter with the same name without a collision. Defining both lets you
provide a read/write property.

## Operators

Wren has most of the same operators you know and love with the same precedence
and associativity. We have three prefix operators:

<pre class="snippet">
! ~ -
</pre>

They are just method calls on their operand without any other arguments. An
expression like `!possible` means "call the `!` method on `possible`".

We also have a slew of infix operators&mdash;they have operands on both sides.
They are:

<pre class="snippet">
* / % + - .. ... << >> < <= > >= == != & ^ | is
</pre>

Like prefix operators, they are all funny ways of writing method calls. The left
operand is the receiver, and the right operand gets passed to it. So `a + b` is
semantically interpreted as "invoke the `+(_)` method on `a`, passing it `b`".

Note that `-` is both a prefix and an infix operator. Since they have different
signatures (`-` and `-(_)`), there's no ambiguity between them.

Most of these are probably familiar already. The `..` and `...` operators are
"range" operators. The number type implements those to create [range][]
objects, but they are method calls like other operators.

[range]: values.html#ranges

The `is` keyword is a "type test" operator. The base [Object][] class implements
it to tell if an object is an instance of a given class. You'll rarely need to,
but you can override `is` in your own classes. That can be useful for things
like mocks or proxies where you want an object to masquerade as a certain class.

[object]: modules/core/object.html

## Subscripts

Another familiar syntax from math is *subscripting* using square brackets
(`[]`). It's handy for working with collection-like objects. For example:

<pre class="snippet">
list[0]    // Get the first item in a list.
map["key"] // Get the value associated with "key".
</pre>

You know the refrain by now. In Wren, these are method calls. In the above
examples, the signature is `[_]`. Subscript operators may also take multiple
arguments, which is useful for things like multi-dimensional arrays:

<pre class="snippet">
matrix[3, 5]
</pre>

These examples are subscript "getters", and there are also
corresponding *subscript setters*:

<pre class="snippet">
list[0] = "item"
map["key"] = "value"
</pre>

These are equivalent to method calls whose signature is `[_]=(_)` and whose
arguments are both the subscript (or subscripts) and the value on the right-hand
side.

## Returning values

The body of a method is a [block](syntax.html#blocks). If it is a single
expression&mdash;more precisely if there is no newline after the `{` &mdash;
then the function implicitly returns the value of the expression.

Otherwise, the body returns `null` by default. You can explicitly return a
value using a `return` statement. In other words, these two methods do the
same thing:

<pre class="snippet">
method1() { "return value" }

method2() {
  return "return value"
}
</pre>


However, it would be an error to include a `return` statment in method1():
<pre class="snippet">
method1() { return "return value" }  //> Error at 'return': Expected expression. 
</pre>

<br><hr>
<a class="right" href="control-flow.html">Control Flow &rarr;</a>
<a href="maps.html">&larr; Maps</a>
