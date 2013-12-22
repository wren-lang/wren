class Foo {
  methodOnFoo { IO.write("foo") }
  method(a) { IO.write("foo") }
  method(a, b, c) { IO.write("foo") }
  override { IO.write("foo") }
}

class Bar is Foo {
  methodOnBar { IO.write("bar") }
  method(a, b) { IO.write("bar") }
  method(a, b, c, d) { IO.write("bar") }
  override { IO.write("bar") }
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
