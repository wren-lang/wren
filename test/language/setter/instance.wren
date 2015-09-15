class Foo {
  construct new() {}

  bar=(value) {
    System.print(value)
  }
}

var foo = Foo.new()
foo.bar = "value" // expect: value
