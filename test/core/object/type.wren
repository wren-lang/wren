class Foo {}

// Object's class is a class.
IO.print(Object is Class) // expect: true

// Its metatype is also a class.
IO.print(Object.type is Class) // expect: true

// The metatype's metatype is Class.
IO.print(Object.type.type == Class) // expect: true

// Object has a distinct metaclass.
IO.print(Object.type.name) // expect: Object metaclass
