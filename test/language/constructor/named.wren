class Foo {
  def construct named() { _field = "named" }
  def construct other() { _field = "other" }

  def toString { _field }
}

System.print(Foo.named()) // expect: named
System.print(Foo.other()) // expect: other

// Returns the new instance.
var foo = Foo.named()
System.print(foo is Foo) // expect: true
System.print(foo.toString) // expect: named
