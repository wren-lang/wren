class Foo {
  methodOnFoo { System.print("foo") }
  method(a) { System.print("foo") }
  method(a, b, c) { System.print("foo") }
  override { System.print("foo") }
}

class Bar is Foo {
  construct new() {}
  methodOnBar { System.print("bar") }
  method(a, b) { System.print("bar") }
  method(a, b, c, d) { System.print("bar") }
  override { System.print("bar") }
}

var bar = Bar.new()
bar.methodOnFoo // expect: foo
bar.methodOnBar // expect: bar

// Methods with different arity do not shadow each other.
bar.method(1) // expect: foo
bar.method(1, 2) // expect: bar
bar.method(1, 2, 3) // expect: foo
bar.method(1, 2, 3, 4) // expect: bar
bar.override // expect: bar
