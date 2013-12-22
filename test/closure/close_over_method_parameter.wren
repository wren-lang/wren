var f = null

class Foo {
  method(param) {
    f = fn {
      IO.write(param)
    }
  }
}

(new Foo).method("param")
f.call // expect: param
