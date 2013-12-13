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

// TODO(bob): Super constructor calls.
// TODO(bob): Super operator calls.
// TODO(bob): Calling super outside of a class.
// TODO(bob): Super calls inside nested functions in methods.
// TODO(bob): Super where there is no inherited method.
