class Base {
  foo { System.print("Base.foo") }
  foo(a) { System.print("Base.foo(a)") }
  foo(a, b) { System.print("Base.foo(a, b)") }
}

class Derived is Base {
  construct new() {}

  foo(a) {
    System.print("Derived.bar(a)")
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
