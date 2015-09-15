class Foo {}

// Object's class is a class.
System.print(Object is Class) // expect: true

// Its metatype is also a class.
System.print(Object.type is Class) // expect: true

// The metatype's metatype is Class.
System.print(Object.type.type == Class) // expect: true

// Object has a distinct metaclass.
System.print(Object.type.name) // expect: Object metaclass
