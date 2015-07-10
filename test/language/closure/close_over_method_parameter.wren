var F = null

class Foo {
  method(param) {
    F = Fn.new {
      IO.print(param)
    }
  }
}

Foo.new().method("param")
F.call() // expect: param
