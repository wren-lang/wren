class Foo {
  toString { return "Foo" }
}

// Classes inherit the argument-less "new" one by default.
var foo = new Foo
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
