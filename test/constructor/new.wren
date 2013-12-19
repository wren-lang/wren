class Foo {
  new { io.write("zero") }
  new(a) { io.write(a) }
  new(a, b) { io.write(a + b) }

  toString { return "Foo" }
}

// Can overload by arity.
new Foo // expect: zero
new Foo("one") // expect: one
new Foo("one", "two") // expect: onetwo

// Returns the new instance.
var foo = new Foo // expect: zero
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
