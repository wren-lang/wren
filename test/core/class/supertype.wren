class Foo {}

class Bar is Foo {}

class Baz is Bar {}

// A class with no explicit superclass inherits Object.
IO.print(Foo.supertype == Object) // expect: true

// Otherwise, it's the superclass.
IO.print(Bar.supertype == Foo) // expect: true
IO.print(Baz.supertype == Bar) // expect: true

// Object has no supertype.
IO.print(Object.supertype) // expect: null
