class Foo {
  toString { return "Foo" }
}

// A class with no constructors gets an argument-less "new" one by default.
var foo = Foo.new
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo

// TODO(bob): Test that class doesn't get default constructor if it has an
// explicit one.
