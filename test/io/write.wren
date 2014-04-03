class Foo {
  toString { "Foo.toString" }
}

// Calls toString on argument.
IO.print(new Foo) // expect: Foo.toString

// Returns argument.
var result = IO.print(123) // expect: 123
IO.print(result == 123) // expect: true

// TODO: What if toString on argument doesn't return a string?
