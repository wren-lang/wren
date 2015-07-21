class Foo {
  construct new() {
    IO.print("ok")
  }
}

var foo = Foo.new() // expect: ok
foo.new() // expect runtime error: Foo does not implement 'new()'.
