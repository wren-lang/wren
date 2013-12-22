var global = "global"
// TODO: Forward reference to global declared after use.

class Foo {
  method {
    IO.write(global)
  }

  static classMethod {
    IO.write(global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
