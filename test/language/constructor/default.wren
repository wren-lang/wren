class Foo {
  toString { "Foo" }
}

// Classes get an argument-less "new()" by default.
var foo = Foo.new()
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
