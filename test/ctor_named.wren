class Foo {
  this bar { io.write("this bar") }
  this baz { io.write("this baz") }
  this bar(arg) { io.write("this bar " + arg) }

  toString { "Foo" }
}

// Different names.
Foo.bar // expect: this bar
Foo.baz // expect: this baz

// Can overload by arity.
Foo.bar // expect: this bar
Foo.bar("one") // expect: this bar one

// Returns the new instance.
var foo = Foo.bar // expect: this bar
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
