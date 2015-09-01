class Base {
  foo {
    IO.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  foo {
    IO.print("Derived.foo")
    super.foo
  }
}

Derived.new().foo
// expect: Derived.foo
// expect: Base.foo
