var Global = "global"

class Foo {
  construct new() {}

  method {
    IO.print(Global)
  }

  static classMethod {
    IO.print(Global)
  }
}

Foo.new().method // expect: global
Foo.classMethod // expect: global
