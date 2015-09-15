class Foo {
  construct new() {}

  method {
    System.print(Global)
  }

  static classMethod {
    System.print(Global)
  }
}

var Global = "global"

Foo.new().method // expect: global
Foo.classMethod // expect: global
