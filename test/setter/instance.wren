class Foo {
  bar = value {
    IO.print(value)
  }
}

var foo = new Foo
foo.bar = "value" // expect: value
