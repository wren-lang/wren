class Foo {
  static bar=(value) {
    System.print(value)
  }
}

Foo.bar = "value" // expect: value
