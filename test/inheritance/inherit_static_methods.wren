class Foo {
  static methodOnFoo { IO.write("foo") }
  static method(a) { IO.write("foo") }
  static method(a, b, c) { IO.write("foo") }
  static override { IO.write("foo") }
}

class Bar is Foo {
  static methodOnBar { IO.write("bar") }
  static method(a, b) { IO.write("bar") }
  static method(a, b, c, d) { IO.write("bar") }
  static override { IO.write("bar") }
}

Bar.methodOnFoo // expect: foo
Bar.methodOnBar // expect: bar

// Methods with different arity do not shadow each other.
Bar.method(1) // expect: foo
Bar.method(1, 2) // expect: bar
Bar.method(1, 2, 3) // expect: foo
Bar.method(1, 2, 3, 4) // expect: bar
Bar.override // expect: bar
