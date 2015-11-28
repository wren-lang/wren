var F

class Foo {
  construct new() {}

  method(param) {
    F = fn () {
      System.print(param)
    }
  }
}

Foo.new().method("param")
F() // expect: param
