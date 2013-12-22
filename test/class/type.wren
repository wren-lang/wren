class Foo {}

// A class is a class.
IO.write(Foo is Class) // expect: true

// It's metatype is also a class.
IO.write(Foo.type is Class) // expect: true

// The metatype's metatype is Class.
IO.write(Foo.type.type == Class) // expect: true
