class Foo {
  construct new() {
    method { "block" }
  }

  method(a) {
    System.print("method %(a())")
  }
}

Foo.new() // expect: method block
