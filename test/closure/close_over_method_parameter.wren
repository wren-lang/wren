var f = null

class Foo {
  method(param) {
    f = fn {
      IO.print(param)
    }
  }
}

(new Foo).method("param")
f.call // expect: param
