class Foo {
  construct new() {}
  bar=(value) { "result" }
}

var foo = Foo.new()
IO.print(foo.bar = "value") // expect: result
