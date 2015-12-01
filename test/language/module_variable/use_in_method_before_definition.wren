class Foo {
  construct new() {}

  method {
    System.print(variable)
  }

  static classMethod {
    System.print(variable)
  }
}

var variable = "module"

Foo.new().method // expect: module
Foo.classMethod // expect: module
