class Foo {
  // TODO(bob): Do we want to require an explicit "new" here?
  this new { io.write("zero") }
  this new(a) { io.write(a) }
  this new(a, b) { io.write(a + b) }

  toString { return "Foo" }
}

// Can overload by arity.
Foo.new // expect: zero
Foo.new("one") // expect: one
Foo.new("one", "two") // expect: onetwo

// Returns the new instance.
var foo = Foo.new // expect: zero
io.write(foo is Foo) // expect: true
io.write(foo.toString) // expect: Foo
