var F = null

class Foo {
  construct new() {}

  method(param) {
    F = Fn.new {
      IO.print(param)
    }
  }
}

Foo.new().method("param")
F.call() // expect: param
