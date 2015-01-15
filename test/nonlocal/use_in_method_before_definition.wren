class Foo {
  method {
    IO.print(Global)
  }

  static classMethod {
    IO.print(Global)
  }
}

var Global = "global"

(new Foo).method // expect: global
Foo.classMethod // expect: global
