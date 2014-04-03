var f = null

class Foo {
  method(param) {
    f = new Fn {
      IO.print(param)
    }
  }
}

(new Foo).method("param")
f.call // expect: param
