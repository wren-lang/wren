class Base {
  def foo {
    System.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  def bar {
    System.print("Derived.bar")
    super.foo
  }
}

Derived.new().bar
// expect: Derived.bar
// expect: Base.foo

// TODO: Super operator calls.
// TODO: Super setter calls.
