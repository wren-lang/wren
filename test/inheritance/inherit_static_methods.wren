class Foo {
  static methodOnFoo { IO.print("foo") }
  static method(a) { IO.print("foo") }
  static method(a, b, c) { IO.print("foo") }
  static override { IO.print("foo") }
}

class Bar is Foo {
  static methodOnBar { IO.print("bar") }
  static method(a, b) { IO.print("bar") }
  static method(a, b, c, d) { IO.print("bar") }
  static override { IO.print("bar") }
}

Bar.methodOnFoo // expect: foo
Bar.methodOnBar // expect: bar

// Methods with different arity do not shadow each other.
Bar.method(1) // expect: foo
Bar.method(1, 2) // expect: bar
Bar.method(1, 2, 3) // expect: foo
Bar.method(1, 2, 3, 4) // expect: bar
Bar.override // expect: bar
