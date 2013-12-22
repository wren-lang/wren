class Foo {
  bar = value { return "result" }
}

var foo = new Foo
IO.write(foo.bar = "value") // expect: result
