
class Foo {
  construct new() { }
}

class Secret {
}

var foo = Foo.new()

System.print(foo.isFrozen)                // expect: false
System.print(foo.setFrozen(true, Secret)) // expect: true
System.print(foo.isFrozen)                // expect: true

// Check `setFrozen` can be performed multiple times
System.print(foo.setFrozen(true, Secret)) // expect: true
System.print(foo.isFrozen)                // expect: true

// Check `setFrozen` can be unset
System.print(foo.setFrozen(false, Secret)) // expect: false
System.print(foo.isFrozen)                 // expect: false
