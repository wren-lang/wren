^title Values

Values are the built-in object types that all other objects are composed of.
They can be created through *literals*, expressions that evaluate to a value.

## Booleans

A boolean value represents truth or falsehood. There are two boolean literals,
`true` and `false`. Its class is `Bool`.

## Numbers

Like other scripting languages, Wren has a single numeric type: double-precision floating point. Number literals look like you expect coming from other languages:

    :::dart
    0
    1234
    -5678
    3.14159
    1.0
    -12.34

Numbers are instances of the `Num` class.

## Strings

Strings are chunks of text. String literals are surrounded in double quotes:

    :::dart
    "hi there"

A couple of escape characters are supported:

    :::dart
    "\n" // Newline.
    "\"" // A double quote character.
    "\\" // A backslash.

Their class is `String`.

## Ranges

A range is a little object that represents a consecutive range of numbers. They don't have their own dedicated literal syntax. Instead, the number class implements `..` and `...` operators to create them:

    :::dart
    3..8

This creates a range from three two eight, including eight itself. If you want a half-inclusive range, use `...`:

    :::dart
    4..6

This creates a range from four to six *not* including six itself. Ranges are commonly used for [looping](looping.html) over a sequences of numbers, but are useful in other places too. You can pass them to a [list](lists.html)'s subscript operator to return a subset of the list, for example:

    var list = ["a", "b", "c", "d", "e"]
    var slice = list[1..3]
    IO.print(slice) // ["b", "c", "d"]

## Null

Wren has a special value `null`, which is the only instance of the class `Null`.
(Note the difference in case.) It functions a bit like `void` in some
languages: it indicates the absence of a value. If you call a method that
doesn't return anything and get its returned value, you get `null` back.
