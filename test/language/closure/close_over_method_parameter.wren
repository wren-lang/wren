var f

class Foo {
  construct new() {}

  method(param) {
    f = Fn.new {
      System.print(param)
    }
  }
}

Foo.new().method("param")
f() // expect: param
