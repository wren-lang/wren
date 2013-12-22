class Foo {
  new { IO.write("zero") }
  new(a) { IO.write(a) }
  new(a, b) { IO.write(a + b) }

  toString { return "Foo" }
}

// Can overload by arity.
new Foo // expect: zero
new Foo("one") // expect: one
new Foo("one", "two") // expect: onetwo

// Returns the new instance.
var foo = new Foo // expect: zero
IO.write(foo is Foo) // expect: true
IO.write(foo.toString) // expect: Foo
