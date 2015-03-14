var F = null

class Foo {
  method(param) {
    F = new Fn {
      IO.print(param)
    }
  }
}

(new Foo).method("param")
F.call() // expect: param
