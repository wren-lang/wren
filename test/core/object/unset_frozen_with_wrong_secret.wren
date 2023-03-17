
class Foo {
  construct new() { }
}

class Secret {
}

class WrongSecret {
}

var foo = Foo.new()

System.print(foo.setFrozen(true, Secret)) // expect: true

// Check `setFrozen` cannot be changed with the wrong secret
System.print(foo.setFrozen(false, WrongSecret)) // expect: true
System.print(foo.isFrozen)                      // expect: true

// Check `setFrozen` can be unset with the secret
System.print(foo.setFrozen(false, Secret)) // expect: false
System.print(foo.isFrozen)                 // expect: false
