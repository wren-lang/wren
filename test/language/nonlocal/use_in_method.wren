var Global = "global"

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
