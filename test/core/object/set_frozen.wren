
class Foo {
  construct new() { }
}

var foo = Foo.new()

System.print(foo.isFrozen)        // expect: false
System.print(foo.setFrozen(true)) // expect: true
System.print(foo.isFrozen)        // expect: true

// Check `setFrozen` can be repeated without error
System.print(foo.setFrozen(true)) // expect: true
System.print(foo.isFrozen)        // expect: true

// Check `setFrozen` can be unset
System.print(foo.setFrozen(false)) // expect: false
System.print(foo.isFrozen)         // expect: false
