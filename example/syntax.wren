// This file provides examples of syntactic constructs in wren, which is mainly
// interesting for testing syntax highlighters.

// This is a comment.
/* This is /* a nested */ comment. */

// Class definition with a toplevel name.
class SyntaxExample {
  // Constructor
  def construct new() {
    // Top-level name `IO`
    System.print("I am a constructor!")

    // Method calls
    variables
    fields()

    // Block arguments
    fields { block }
    fields {|a, b| block }
    fields(argument) { block }
    fields(argument) {|a, b| block }

    // Static method call
    SyntaxExample.fields(1)
  }

  // Constructor with arguments
  def construct constructor(a, b) {
    print(a, b)
    field = a
  }

  // Method without arguments
  def variables {
    // Valid local variable names.
    var hi
    var camelCase
    var PascalCase
    var abc123
    var ALL_CAPS
  }

  // Method with empty argument list
  def fields() {
    // Fields
    _under_score = 1
    _field = 2
  }

  // Static method with single argument
  def static fields(a) {
    // Static field
    __a = a
  }

  // Setter
  def field=(value) { _field = value }

  // Method with arguments
  def print(a, b) { System.print(a + b) }

  // Operators
  def +(other) { "infix + %(other)" }
  def -(other) { "infix - %(other)" }
  def *(other) { "infix * %(other)" }
  def /(other) { "infix / %(other)" }
  def %(other) { "infix \% %(other)" }
  def <(other) { "infix < %(other)" }
  def >(other) { "infix > %(other)" }
  def <=(other) { "infix <= %(other)" }
  def >=(other) { "infix >= %(other)" }
  def ==(other) { "infix == %(other)" }
  def !=(other) { "infix != %(other)" }
  def &(other) { "infix & %(other)" }
  def |(other) { "infix | %(other)" }

  def ! { "prefix !" }
  def ~ { "prefix ~" }
  def - { "prefix -" }
}

// `class`, `is`
class ReservedWords is SyntaxExample {
  def reserved {
    // `super`, `true`, `false`
    super(true, false)
    // `this`
    this.foo
  }

  def foo {
    // `var`
    var n = 27
    // `while`, `if`, `else`
    while (n != 1) if (n % 2 == 0) n = n / 2 else n = 3 * n + 1

    // `for`, `in`
    for (beatle in ["george", "john", "paul", "ringo"]) {
      System.print(beatle)
      // `break`
      break
    }

    // `return`, `null`
    return null
  }

  def imports {
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

class Literals is SyntaxExample {
  def booleans { true || false }

  def numbers {
    0
    1234
    -5678
    3.14159
    1.0
    -12.34
    0xdeadbeef
    0x1234567890ABCDEF
  }

  def strings {
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
    System.print("\u0041fgdg\u0b83\u00DE") // "AஃÞ"
    // Unencoded bytes
    System.print("\x48\x69\x2e") // "Hi."
  }

  def ranges {
    3..8  // inclusive
    4...6 // half-inclusive
  }

  def nothing { null }

  def lists {
    var list = [1, "banana", true]
    list[0] = 5
    list[1..2]
  }

  def maps {
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
