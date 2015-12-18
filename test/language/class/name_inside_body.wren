class Foo {
  def construct new() {}

  def static sayName {
    System.print(Foo)
  }

  def sayName {
    System.print(Foo)
  }

  def static toString { "Foo!" }
}

Foo.sayName // expect: Foo!
Foo.new().sayName // expect: Foo!
