class Foo {
  static sayName {
    io.write(Foo.toString)
  }

  sayName {
    io.write(Foo.toString)
  }

  static toString { return "Foo!" }
}

Foo.sayName // expect: Foo!
(new Foo).sayName // expect: Foo!
