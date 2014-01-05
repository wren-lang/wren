class Foo {}

// A class is a class.
IO.print(Foo is Class) // expect: true

// It's metatype is also a class.
IO.print(Foo.type is Class) // expect: true

// The metatype's metatype is Class.
IO.print(Foo.type.type == Class) // expect: true
