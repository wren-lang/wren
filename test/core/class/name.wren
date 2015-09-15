class Foo {}

System.print(Foo.name) // expect: Foo
System.print(Foo.type.name) // expect: Foo metaclass

// Make sure the built-in classes have proper names too.
System.print(Object.name) // expect: Object
System.print(Bool.name) // expect: Bool
System.print(Class.name) // expect: Class

// And metaclass names.
System.print(Object.type.name) // expect: Object metaclass
System.print(Bool.type.name) // expect: Bool metaclass
