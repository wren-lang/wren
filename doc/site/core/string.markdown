^title String Class
^category core

Strings are immutable chunks of text. More formally, a string is a sequence of
Unicode code points encoded in UTF-8.

If you never work with any characters outside of the ASCII range, you can treat
strings like a directly indexable array of characters. Once other characters
get involved, it's important to understand the distinction.

In UTF-8, a single Unicode code point (very roughly a single "character") may
be encoded as one or more bytes. This means you can't directly index by code
point. There's no way to find, say, the fifth code unit in a string without
walking the string from the beginning and counting them as you go.

Because counting code units is relatively slow, string methods generally index
by *byte*, not *code unit*. When you do:

    :::dart
    someString[3]

That means "get the code unit starting at *byte* three", not "get the third
code unit in the string". This sounds scary, but keep in mind that the methods
on string *return* byte indices too. So, for example, this does what you want:

    :::dart
    var metalBand = "Fäcëhämmër"
    var hPosition = metalBand.indexOf("h")
    IO.print(metalBand[hPosition]) // "h"

In general, methods on strings will work in terms of code units if they can do
so efficiently, and will otherwise deal in bytes.

## Static Methods

### String.**fromCodePoint**(codePoint)

Creates a new string containing the UTF-8 encoding of `codePoint`.

It is a runtime error if `codePoint` is not an integer between `0` and
`0x10ffff`, inclusive.

    :::dart
    String.fromCodePoint(8225) // "‡"

## Methods

### **contains**(other)

Checks if `other` is a substring of the string.

It is a runtime error if `other` is not a string.

### **count**

Returns the length of the string.

### **endsWith**(suffix)

Checks if the string ends with `suffix`.

It is a runtime error if `suffix` is not a string.

### **indexOf**(search)

Returns the index of the first byte matching `search` in the string or `-1` if
`search` was not found.

It is a runtime error if `search` is not a string.

### **iterate**(iterator), **iteratorValue**(iterator)

Implements the [iterator protocol](../control-flow.html#the-iterator-protocol)
for iterating over the *code points* in the string:

    :::dart
    var codePoints = []
    for (c in "(ᵔᴥᵔ)") {
      codePoints.add(c)
    }

    IO.print(codePoints) // ["(", "ᵔ", "ᴥ", "ᵔ", ")"].

### **startsWith**(prefix)

Checks if the string starts with `prefix`.

It is a runtime error if `prefix` is not a string.

### **+**(other) operator

Returns a new string that concatenates this string and `other`.

It is a runtime error if `other` is not a string.

### **==**(other) operator

Checks if the string is equal to `other`.

### **!=**(other) operator

Check if the string is not equal to `other`.

### **[**index**]** operator

Returns a string containing the code unit starting at byte `index`.

    :::dart
    IO.print("ʕ•ᴥ•ʔ"[5]) // "ᴥ".

Since `ʕ` is two bytes in UTF-8 and `•` is three, the fifth byte points to the
bear's nose.

If `index` points into the middle of a UTF-8 sequence, this returns an empty
string:

    :::dart
    IO.print("I ♥ NY"[3]) // "".

It is a runtime error if `index` is greater than the number of bytes in the
string.
