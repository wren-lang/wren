var Global = "global"
// TODO: Forward reference to global declared after use.

class Foo {
  method {
    IO.print(Global)
  }

  static classMethod {
    IO.print(Global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
