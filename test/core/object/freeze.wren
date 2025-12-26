
class Foo {
  construct new() { }
}

var foo = Foo.new()

System.print(foo.isFrozen) // expect: false
System.print(foo.freeze()) // expect: true
System.print(foo.isFrozen) // expect: true

// Check `freeze` can be repeated without error
System.print(foo.freeze()) // expect: true
System.print(foo.isFrozen) // expect: true
