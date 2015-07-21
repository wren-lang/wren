class Foo {
  construct new() {
    IO.print("Foo.new()")
  }
}

class Bar is Foo {}

Bar.new() // expect: Foo.new()
