var variable = "module"

class Foo {
  construct new() {}

  method {
    System.print(variable)
  }

  static classMethod {
    System.print(variable)
  }
}

Foo.new().method // expect: module
Foo.classMethod // expect: module
