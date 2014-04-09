class Foo {
  static sayName {
    IO.print(Foo)
  }

  sayName {
    IO.print(Foo)
  }

  static toString { "Foo!" }
}

Foo.sayName // expect: Foo!
(new Foo).sayName // expect: Foo!
