class Foo {
  toString { "Foo" }
}

// Classes inherit the argument-less "new" one by default.
var foo = new Foo
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
