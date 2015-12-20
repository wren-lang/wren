class Foo {
  construct new() {
    System.print("constructor %(_field)")
  }

  System.print("before %(_field)")

  _field = "initialized"

  System.print("after %(_field)")
}

Foo.new()
// expect: before null
// expect: after initialized
// expect: constructor initialized
