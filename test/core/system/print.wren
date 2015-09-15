class Foo {
  construct new() {}

  toString { "Foo.toString" }
}

// Calls toString on argument.
System.print(Foo.new()) // expect: Foo.toString

// Returns the argument.
System.print(System.print(1) == 1) // expect: 1
                                   // expect: true
