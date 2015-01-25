^title Values
^category types

Values are the built-in object types that all other objects are composed of.
They can be created through *literals*, expressions that evaluate to a value.
All values are *immutable*&mdash;once created, they do not change. The number
`3` is always the number `3`. The string `"frozen"` can never have its
character array modified in place.

## Booleans

A boolean value represents truth or falsehood. There are two boolean literals,
`true` and `false`. Their class is [Bool](core/bool.html).

## Numbers

Like other scripting languages, Wren has a single numeric type:
double-precision floating point. Number literals look like you expect coming
from other languages:

    :::dart
    0
    1234
    -5678
    3.14159
    1.0
    -12.34

Numbers are instances of the [Num](core/num.html) class.

## Strings

Strings are chunks of text stored as UTF-8. Their class is
[String](core/string.html). String literals are surrounded in double quotes:

    :::dart
    "hi there"

A handful of escape characters are supported:

    :::dart
    "\"" // A double quote character.
    "\\" // A backslash.
    "\a" // Alarm beep. (Who uses this?)
    "\b" // Backspace.
    "\f" // Formfeed.
    "\n" // Newline.
    "\r" // Carriage return.
    "\t" // Tab.
    "\v" // Vertical tab.

A `\u` followed by four hex digits can be used to specify a Unicode code point.

## Ranges

A range is a little object that represents a consecutive range of integers.
They don't have their own dedicated literal syntax. Instead, the number class
implements the `..` and `...` [operators](expressions.html#operators) to create
them:

    :::dart
    3..8

This creates a range from three to eight, including eight itself. If you want a
half-inclusive range, use `...`:

    :::dart
    4...6

This creates a range from four to six *not* including six itself. Ranges are
commonly used for [iterating](control-flow.html#for-statements) over a
sequences of numbers, but are useful in other places too. You can pass them to
a [list](lists.html)'s subscript operator to return a subset of the list, for
example:

    :::dart
    var list = ["a", "b", "c", "d", "e"]
    var slice = list[1..3]
    IO.print(slice) // ["b", "c", "d"]

Their class is [Range](core/range.html)

## Null

Wren has a special value `null`, which is the only instance of the class
[Null](core/null.html). (Note the difference in case.) It functions a bit like
`void` in some languages: it indicates the absence of a value. If you call a
method that doesn't return anything and get its returned value, you get `null`
back.
