class Foo {
  construct new() {}

  static sayName {
    System.print(Foo)
  }

  sayName {
    System.print(Foo)
  }

  static toString { "Foo!" }
}

Foo.sayName // expect: Foo!
Foo.new().sayName // expect: Foo!
