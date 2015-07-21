class Foo {
  construct named() { _field = "named" }
  construct other() { _field = "other" }

  toString { _field }
}

IO.print(Foo.named()) // expect: named
IO.print(Foo.other()) // expect: other

// Returns the new instance.
var foo = Foo.named()
IO.print(foo is Foo) // expect: true
IO.print(foo.toString) // expect: named
