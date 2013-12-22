class Foo {
  bar = value {
    IO.write(value)
  }
}

var foo = new Foo
foo.bar = "value" // expect: value
