class Foo {
  static sayName {
    IO.print(Foo.toString)
  }

  sayName {
    IO.print(Foo.toString)
  }

  static toString { return "Foo!" }
}

Foo.sayName // expect: Foo!
(new Foo).sayName // expect: Foo!
