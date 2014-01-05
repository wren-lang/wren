class Foo {
  static bar = value {
    IO.print(value)
  }
}

Foo.bar = "value" // expect: value
