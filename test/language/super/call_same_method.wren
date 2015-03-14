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

(new Derived).foo
// expect: Derived.foo
// expect: Base.foo
