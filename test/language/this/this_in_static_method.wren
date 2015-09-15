class Foo {
  static test {
    System.print(this == Foo) // expect: true
    System.print(this.bar) // expect: bar
  }

  static bar { "bar" }
}

Foo.test
