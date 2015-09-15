class Base {
  foo {
    System.print("Base.foo")
  }
}

class Derived is Base {
  construct new() {}

  foo {
    System.print("Derived.foo")
    super
  }
}

Derived.new().foo
// expect: Derived.foo
// expect: Base.foo
