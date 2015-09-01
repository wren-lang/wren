class Base {
  foo {
    IO.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  bar {
    IO.print("Derived.bar")
    super.foo
  }
}

Derived.new().bar
// expect: Derived.bar
// expect: Base.foo

// TODO: Super operator calls.
// TODO: Super setter calls.
