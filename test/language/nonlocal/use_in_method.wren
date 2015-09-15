var Global = "global"

class Foo {
  construct new() {}

  method {
    System.print(Global)
  }

  static classMethod {
    System.print(Global)
  }
}

Foo.new().method // expect: global
Foo.classMethod // expect: global
