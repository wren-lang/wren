class Foo {}

// A class is a class.
System.print(Foo is Class) // expect: true

// Its metatype is also a class.
System.print(Foo.type is Class) // expect: true

// The metatype's metatype is Class.
System.print(Foo.type.type == Class) // expect: true

// And Class's metatype circles back onto itself.
System.print(Foo.type.type.type == Class) // expect: true
System.print(Foo.type.type.type.type == Class) // expect: true
System.print(Foo.type.type.type.type.type == Class) // expect: true
