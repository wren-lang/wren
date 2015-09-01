class Foo {
  construct new() {}

  bar=(value) {
    IO.print(value)
  }
}

var foo = Foo.new()
foo.bar = "value" // expect: value
