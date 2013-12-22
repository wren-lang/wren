class Foo {
  static sayName {
    IO.write(Foo.toString)
  }

  sayName {
    IO.write(Foo.toString)
  }

  static toString { return "Foo!" }
}

Foo.sayName // expect: Foo!
(new Foo).sayName // expect: Foo!
