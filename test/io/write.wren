class Foo {
  toString { return "Foo.toString" }
}

// Calls toString on argument.
IO.write(new Foo) // expect: Foo.toString

// Returns argument.
var result = IO.write(123) // expect: 123
IO.write(result == 123) // expect: true

// TODO: What if toString on argument doesn't return a string?
