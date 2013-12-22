class Foo {
  toString { return "Foo" }
}

// Classes inherit the argument-less "new" one by default.
var foo = new Foo
IO.write(foo is Foo) // expect: true
IO.write(foo.toString) // expect: Foo
