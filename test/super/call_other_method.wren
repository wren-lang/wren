class Base {
  foo {
    IO.print("Base.foo")
  }
}

class Derived is Base {
  bar {
    IO.print("Derived.bar")
    super.foo
  }
}

(new Derived).bar
// expect: Derived.bar
// expect: Base.foo

// TODO: Super operator calls.
// TODO: Super setter calls.
