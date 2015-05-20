// This file provides examples of syntactic constructs in wren, which is mainly
// interesting for testing syntax highlighters.

// This is a comment.
/* This is /* a nested */ comment. */

// Class definition with a toplevel name.
class SyntaxExample {
  // Constructor
  new {
    // toplevel name `IO`
    IO.print("I am a constructor!")
    // method calls
    variables
    fields()
    // static method call
    SyntaxExample.fields(1)
  }

  // Constructor with arguments
  new(a, b) {
    print(a, b)
    field = a
  }

  // Method without arguments
  variables {
    // Valid local variable names.
    var hi
    var camelCase
    var PascalCase
    var abc123
    var ALL_CAPS
  }

  // Method with empty argument list
  fields() {
    // Fields
    _under_score = 1
    _field = 2
  }

  // Static method with single argument
  static fields(a) {
    // Static field
    __a = a
  }

  // setter
  field=(value) { _field = value }

  // Method with arguments
  print(a, b) { IO.print(a + b) }
}

// `class`, `is`
class ReservedWords is SyntaxExample {
  // `new`
  new {
    // `super`, `true`, `false`
    super(true, false)
    // `this`
    this.foo
    // `new`
    new SyntaxExample
  }

  foo {
    // `var`
    var n = 27
    // `while`, `if`, `else`
    while (n != 1) if (n % 2 == 0) n = n / 2 else n = 3 * n + 1

    // `for`, `in`
    for (beatle in ["george", "john", "paul", "ringo"]) {
      IO.print(beatle)
      // `break`
      break
    }

    // `return`, `null`
    return null
  }

  imports {
    // `import`
    import "hello"
    // `import`, `for`
    import "set" for Set
  }

  // `foreign`, `static`
  // foreign static bar
  // foreign baz(string)
  // (Remove lines above to make this file compile)
}

class literals is SyntaxExample {
  booleans { true || false }
  numbers {
    0
    1234
    -5678
    3.14159
    1.0
    -12.34
    0xdeadbeef
    0x1234567890ABCDEF
  }
  strings {
    "hi there"
    // Escapes:
    "\0" // The NUL byte: 0.
    "\"" // A double quote character.
    "\\" // A backslash.
    "\a" // Alarm beep. (Who uses this?)
    "\b" // Backspace.
    "\f" // Formfeed.
    "\n" // Newline.
    "\r" // Carriage return.
    "\t" // Tab.
    "\v" // Vertical tab.
    // Unicode code points
    IO.print("\u0041\u0b83\u00DE") // "AஃÞ"
    // Unencoded bytes
    IO.print("\x48\x69\x2e") // "Hi."
  }
  ranges {
    3..8  // inclusive
    4...6 // half-inclusive
  }
  nothing { null }

  lists {
    var list = [1, "banana", true]
    list[0] = 5
    list[1..2]
  }
  maps {
    var stringMap = {
      "George": "Harrison",
      "John": "Lennon",
      "Paul": "McCartney",
      "Ringo": "Starr"
    }
    var a = 1
    var weirdMap = {
      true: 1,
      false: 0,
      null: -1,
      "str": "abc",
      (1..5): 10,
      a: 2,
      _a: 3,
      __a: 4
    }
  }
}
