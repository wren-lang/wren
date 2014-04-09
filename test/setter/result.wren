class Foo {
  bar=(value) { "result" }
}

var foo = new Foo
IO.print(foo.bar = "value") // expect: result
