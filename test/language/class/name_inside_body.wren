class Foo {
  construct new() {}

  static def sayName {
    System.print(Foo)
  }

  def sayName {
    System.print(Foo)
  }

  static def toString { "Foo!" }
}

Foo.sayName // expect: Foo!
Foo.new().sayName // expect: Foo!
