class Base {
  def foo {
    System.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  def foo {
    System.print("Derived.foo")
    super
  }
}

Derived.new().foo
// expect: Derived.foo
// expect: Base.foo
