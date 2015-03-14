class Foo {
  new { IO.print("none") }
  new() { IO.print("zero") }
  new(a) { IO.print(a) }
  new(a, b) { IO.print(a + b) }

  toString { "Foo" }
}

// Can overload by arity.
new Foo // expect: none
new Foo() // expect: zero
new Foo("one") // expect: one
new Foo("one", "two") // expect: onetwo

// Returns the new instance.
var foo = new Foo // expect: none
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: Foo
