class Foo {
  bar = value {
    io.write(value)
  }
}

var foo = new Foo
foo.bar = "value" // expect: value
