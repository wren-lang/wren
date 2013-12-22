class Base {
  foo {
    IO.write("Base.foo")
  }
}

class Derived is Base {
  foo {
    IO.write("Derived.foo")
    super.foo
  }
}

(new Derived).foo
// expect: Derived.foo
// expect: Base.foo
