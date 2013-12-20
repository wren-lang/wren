class Foo {
  bar = value { return "result" }
}

var foo = new Foo
io.write(foo.bar = "value") // expect: result
