class Foo {}

IO.print(Foo.name) // expect: Foo
IO.print(Foo.type.name) // expect: Foo metaclass

// Make sure the built-in classes have proper names too.
IO.print(Object.name) // expect: Object
IO.print(Bool.name) // expect: Bool
IO.print(Class.name) // expect: Class

// And metaclass names.
IO.print(Object.type.name) // expect: Object metaclass
IO.print(Bool.type.name) // expect: Bool metaclass
