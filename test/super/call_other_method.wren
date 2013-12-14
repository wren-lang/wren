class Base {
  foo {
    io.write("Base.foo")
  }
}

class Derived is Base {
  bar {
    io.write("Derived.bar")
    super.foo
  }
}

Derived.new.bar
// expect: Derived.bar
// expect: Base.foo

// TODO: Super constructor calls.
// TODO: Super operator calls.
// TODO: Calling super outside of a class.
// TODO: Super calls inside nested functions in methods.
// TODO: Super where there is no inherited method.
