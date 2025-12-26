
class Foo {
}

// Check core bootstrap classes
System.print(Class.isFrozen)       // expect: false
System.print(Object.isFrozen)      // expect: false
System.print(Object.type.isFrozen) // expect: false

// Check a core class
System.print(List.isFrozen)        // expect: false
System.print(List.type.isFrozen)   // expect: false

// Check a user defined class
System.print(Foo.isFrozen)         // expect: false
System.print(Foo.type.isFrozen)    // expect: false
