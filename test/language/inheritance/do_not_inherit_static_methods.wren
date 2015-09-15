class Foo {
  static methodOnFoo { System.print("foo") }
}

class Bar is Foo {}

Bar.methodOnFoo // expect runtime error: Bar metaclass does not implement 'methodOnFoo'.
