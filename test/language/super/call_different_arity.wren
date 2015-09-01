class Base {
  foo { IO.print("Base.foo") }
  foo(a) { IO.print("Base.foo(a)") }
  foo(a, b) { IO.print("Base.foo(a, b)") }
}

class Derived is Base {
  construct new() {}

  foo(a) {
    IO.print("Derived.bar(a)")
    super
    super(1)
    super(1, 2)
  }
}

Derived.new().foo(1)
// expect: Derived.bar(a)
// expect: Base.foo
// expect: Base.foo(a)
// expect: Base.foo(a, b)
