class Foo {
  toString { return "Foo" }
}

// Classes inherit the argument-less "new" one by default.
var foo = new Foo
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo

// TODO: Get rid of this. If you're defining a class, it's because you have
// some state to initialize. if you don't, it shouldn't be a class.
