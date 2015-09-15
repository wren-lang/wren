class Foo {}

class Bar is Foo {}

class Baz is Bar {}

// A class with no explicit superclass inherits Object.
System.print(Foo.supertype == Object) // expect: true

// Otherwise, it's the superclass.
System.print(Bar.supertype == Foo) // expect: true
System.print(Baz.supertype == Bar) // expect: true

// Object has no supertype.
System.print(Object.supertype) // expect: null
