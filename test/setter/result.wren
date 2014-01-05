class Foo {
  bar = value { return "result" }
}

var foo = new Foo
IO.print(foo.bar = "value") // expect: result
