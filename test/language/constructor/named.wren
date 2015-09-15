class Foo {
  construct named() { _field = "named" }
  construct other() { _field = "other" }

  toString { _field }
}

System.print(Foo.named()) // expect: named
System.print(Foo.other()) // expect: other

// Returns the new instance.
var foo = Foo.named()
System.print(foo is Foo) // expect: true
System.print(foo.toString) // expect: named
