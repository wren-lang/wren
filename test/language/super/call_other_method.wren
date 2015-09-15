class Base {
  foo {
    System.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  bar {
    System.print("Derived.bar")
    super.foo
  }
}

Derived.new().bar
// expect: Derived.bar
// expect: Base.foo

// TODO: Super operator calls.
// TODO: Super setter calls.
