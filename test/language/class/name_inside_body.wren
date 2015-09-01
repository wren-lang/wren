class Foo {
  construct new() {}

  static sayName {
    IO.print(Foo)
  }

  sayName {
    IO.print(Foo)
  }

  static toString { "Foo!" }
}

Foo.sayName // expect: Foo!
Foo.new().sayName // expect: Foo!
