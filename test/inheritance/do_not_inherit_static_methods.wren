class Foo {
  static methodOnFoo { IO.print("foo") }
}

class Bar is Foo {}

Bar.methodOnFoo // expect runtime error: Bar metaclass does not implement method 'methodOnFoo'.
