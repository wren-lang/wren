var f = null

class Foo {
  method(param) {
    f = fn {
      io.write(param)
    }
  }
}

(new Foo).method("param")
f.call // expect: param
