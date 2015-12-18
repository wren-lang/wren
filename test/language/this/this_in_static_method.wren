class Foo {
  def static test {
    System.print(this == Foo) // expect: true
    System.print(this.bar) // expect: bar
  }

  def static bar { "bar" }
}

Foo.test
