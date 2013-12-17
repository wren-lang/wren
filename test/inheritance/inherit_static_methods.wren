class Foo {
  static methodOnFoo { io.write("foo") }
  static method(a) { io.write("foo") }
  static method(a, b, c) { io.write("foo") }
  static override { io.write("foo") }
}

class Bar is Foo {
  static methodOnBar { io.write("bar") }
  static method(a, b) { io.write("bar") }
  static method(a, b, c, d) { io.write("bar") }
  static override { io.write("bar") }
}

Bar.methodOnFoo // expect: foo
Bar.methodOnBar // expect: bar

// Methods with different arity do not shadow each other.
Bar.method(1) // expect: foo
Bar.method(1, 2) // expect: bar
Bar.method(1, 2, 3) // expect: foo
Bar.method(1, 2, 3, 4) // expect: bar
Bar.override // expect: bar
