var F

class Foo {
  construct new() {}

  method(param) {
    F = Fn.new {
      System.print(param)
    }
  }
}

Foo.new().method("param")
F() // expect: param
