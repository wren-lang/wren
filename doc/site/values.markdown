^title Values

Values are the built-in object types that all other objects are composed of.
They can be created through *literals*, expressions that evaluate to a value.

All value types in Wren are immutable. That means that once created, they
cannot be changed. `3` is always three and `"hi"` is always `"hi"`.

## Booleans

A boolean value represents truth or falsehood. There are two boolean literals,
`true` and `false`. Its class is `Bool`.

## Numbers

Like other scripting languages, Wren has a single numeric type: double-precision floating point. Number literals look like you expect coming from other languages:

    :::wren
    0
    1234
    -5678
    3.14159
    1.0
    -12.34

Numbers are instances of the `Num` class.

## Strings

Strings are chunks of text. String literals are surrounded in double quotes:

    :::wren
    "hi there"

A couple of escape characters are supported:

    :::wren
    "\n" // Newline.
    "\"" // A double quote character.
    "\\" // A backslash.

Their class is `String`.

## Null

Wren has a special value `null`, which is the only instance of the class `Null`.
(Note the difference in case.) It functions a bit like `void` in some
languages: it indicates the absence of a value. If you call a method that
doesn't return anything and get its returned value, you get `null` back.
