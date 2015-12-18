class Foo {
  static def bar=(value) {
    System.print(value)
  }
}

Foo.bar = "value" // expect: value
