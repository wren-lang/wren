class Foo {
  static bar = value {
    IO.write(value)
  }
}

Foo.bar = "value" // expect: value
