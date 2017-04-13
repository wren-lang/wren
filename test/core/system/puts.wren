class Foo {
  construct new() {}

  toString { "Foo.toString" }
}

// Calls toString on argument.
System.puts(Foo.new()) // expect: Foo.toString
