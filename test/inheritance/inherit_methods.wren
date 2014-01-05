class Foo {
  methodOnFoo { IO.print("foo") }
  method(a) { IO.print("foo") }
  method(a, b, c) { IO.print("foo") }
  override { IO.print("foo") }
}

class Bar is Foo {
  methodOnBar { IO.print("bar") }
  method(a, b) { IO.print("bar") }
  method(a, b, c, d) { IO.print("bar") }
  override { IO.print("bar") }
}

var bar = new Bar
bar.methodOnFoo // expect: foo
bar.methodOnBar // expect: bar

// Methods with different arity do not shadow each other.
bar.method(1) // expect: foo
bar.method(1, 2) // expect: bar
bar.method(1, 2, 3) // expect: foo
bar.method(1, 2, 3, 4) // expect: bar
bar.override // expect: bar

// TODO: Prevent extending built-in types.
