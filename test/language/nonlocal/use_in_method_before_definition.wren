class Foo {
  method {
    IO.print(Global)
  }

  static classMethod {
    IO.print(Global)
  }
}

var Global = "global"

Foo.new().method // expect: global
Foo.classMethod // expect: global
