class Foo {
  static def test {
    System.print(this == Foo) // expect: true
    System.print(this.bar) // expect: bar
  }

  static def bar { "bar" }
}

Foo.test
