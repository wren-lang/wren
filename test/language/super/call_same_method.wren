class Base {
  foo {
    IO.print("Base.foo")
  }
}

class Derived is Base {
  foo {
    IO.print("Derived.foo")
    super.foo
  }
}

Derived.new().foo
// expect: Derived.foo
// expect: Base.foo
