var f = null

class Foo {
  method(param) {
    f = fn {
      io.write(param)
    }
  }
}

Foo.new.method("param")
f.call // expect: param
