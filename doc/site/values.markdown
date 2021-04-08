^title Values

Values are the built-in atomic object types that all other objects are composed
of. They can be created through *literals*, expressions that evaluate to a
value. All values are *immutable*&mdash;once created, they do not change. The
number `3` is always the number `3`. The string `"frozen"` can never have its
character array modified in place.

## Booleans

A boolean value represents truth or falsehood. There are two boolean literals,
`true` and `false`. Their class is [Bool][].

[bool]: modules/core/bool.html

## Numbers

Like other scripting languages, Wren has a single numeric type:
double-precision floating point. Number literals look like you expect coming
from other languages:


<pre class="snippet">
0
1234
-5678
3.14159
1.0
-12.34
0.0314159e02
0.0314159e+02
314.159e-02
0xcaffe2
</pre>

Numbers are instances of the [Num][] class.

[num]: modules/core/num.html

## Strings

A string is an array of bytes. Typically, they store characters encoded in
UTF-8, but you can put any byte values in there, even zero or invalid UTF-8
sequences. (You might have some trouble *printing* the latter to your terminal,
though.)

String literals are surrounded in double quotes:

<pre class="snippet">
"hi there"
</pre>

They can also span multiple lines:

<pre class="snippet">
"hi
there,
again"
</pre>

### Escaping

A handful of escape characters are supported:

<pre class="snippet">
"\0" // The NUL byte: 0.
"\"" // A double quote character.
"\\" // A backslash.
"\%" // A percent sign.
"\a" // Alarm beep. (Who uses this?)
"\b" // Backspace.
"\e" // ESC character.
"\f" // Formfeed.
"\n" // Newline.
"\r" // Carriage return.
"\t" // Tab.
"\v" // Vertical tab.


"\x48"        // Unencoded byte     (2 hex digits)
"\u0041"      // Unicode code point (4 hex digits)
"\U0001F64A"  // Unicode code point (8 hex digits)
</pre>

A `\x` followed by two hex digits specifies a single unencoded byte:

<pre class="snippet">
System.print("\x48\x69\x2e") //> Hi.
</pre>

A `\u` followed by four hex digits can be used to specify a Unicode code point:

<pre class="snippet">
System.print("\u0041\u0b83\u00DE") //> AஃÞ
</pre>

A capital `\U` followed by *eight* hex digits allows Unicode code points outside
of the basic multilingual plane, like all-important emoji:

<pre class="snippet">
System.print("\U0001F64A\U0001F680") //> 🙊🚀
</pre>

Strings are instances of class [String][].

[string]: modules/core/string.html

### Interpolation

String literals also allow *interpolation*. If you have a percent sign (`%`)
followed by a parenthesized expression, the expression is evaluated. The
resulting object's `toString` method is called and the result is inserted in the
string:

<pre class="snippet">
System.print("Math %(3 + 4 * 5) is fun!") //> Math 23 is fun!
</pre>

Arbitrarily complex expressions are allowed inside the parentheses:

<pre class="snippet">
System.print("wow %((1..3).map {|n| n * n}.join())") //> wow 149
</pre>

An interpolated expression can even contain a string literal which in turn has
its own nested interpolations, but doing that gets unreadable pretty quickly.

### Raw strings

A string literal can also be created using triple quotes `"""` which is
parsed as a raw string. A raw string is no different
from any other string, it's just parsed in a different way.

**Raw strings do not process escapes and do not apply any interpolation**.

<pre class="snippet">
"""hi there"""
</pre>

When a raw string spans multiple lines and a triple quote is on it's own line,
any whitespace on that line will be ignored. This means the opening and closing
lines are not counted as part of the string when the triple quotes are separate lines,
as long as they only contain whitespace (spaces + tabs).

<pre class="snippet">
  """
    Hello world
  """
</pre>

The resulting value in the string above has no newlines or trailing whitespace. 
Note the spaces in front of the Hello are preserved. 

<pre class="snippet">
    Hello world
</pre>

A raw string will be parsed exactly as is in the file, unmodified.
This means it can contain quotes, invalid syntax, other data formats 
and so on without being modified by Wren.

<pre class="snippet">
"""
  {
    "hello": "wren",
    "from" : "json"
  }
"""
</pre>

One more example, embedding wren code inside a string safely.

<pre class="snippet">
"""
A markdown string with embedded wren code example.

    class Example {
      construct code() {
        //
      }
    }
"""
</pre>

## Ranges

A range is a little object that represents a consecutive range of numbers. They
don't have their own dedicated literal syntax. Instead, the number class
implements the `..` and `...` [operators][] to create them:

[operators]: method-calls.html#operators

<pre class="snippet">
3..8
</pre>

This creates a range from three to eight, including eight itself. If you want a
half-inclusive range, use `...`:

<pre class="snippet">
4...6
</pre>

This creates a range from four to six *not* including six itself. Ranges are
commonly used for [iterating](control-flow.html#for-statements) over a
sequences of numbers, but are useful in other places too. You can pass them to
a [list](lists.html)'s subscript operator to return a subset of the list, for
example, or on a String, the substring in that range:

<pre class="snippet">
var list = ["a", "b", "c", "d", "e"]
var slice = list[1..3]
System.print(slice) //> [b, c, d]

var string = "hello wren"
var wren = string[-4..-1]
System.print(wren) //> wren
</pre>

Their class is [Range][].

[range]: modules/core/range.html

## Null

Wren has a special value `null`, which is the only instance of the class
[Null][]. (Note the difference in case.) It functions a bit like `void` in some
languages: it indicates the absence of a value. If you call a method that
doesn't return anything and get its returned value, you get `null` back.

[null]: modules/core/null.html

<br><hr>
<a class="right" href="lists.html">Lists &rarr;</a>
<a href="syntax.html">&larr; Syntax</a>
