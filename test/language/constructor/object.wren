// Tests that Object implements new(). The only way to call that is through a
// super() call in a subclass, so this does that.

class Foo {
  construct new() {
    super() // Should not cause a no method error.
    IO.print("ok")
  }
}

Foo.new() // expect: ok
