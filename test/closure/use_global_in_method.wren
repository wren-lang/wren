var global = "global"
// TODO: Forward reference to global declared after use.

class Foo {
  method {
    io.write(global)
  }

  static classMethod {
    io.write(global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
