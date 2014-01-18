class Foo {
  static methodOnFoo { IO.print("foo") }
}

class Bar is Foo {}

Bar.methodOnFoo // expect runtime error: Receiver does not implement method 'methodOnFoo'.
