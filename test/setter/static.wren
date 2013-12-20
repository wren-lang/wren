class Foo {
  static bar = value {
    io.write(value)
  }
}

Foo.bar = "value" // expect: value
