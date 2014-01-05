var global = "global"
// TODO: Forward reference to global declared after use.

class Foo {
  method {
    IO.print(global)
  }

  static classMethod {
    IO.print(global)
  }
}

(new Foo).method // expect: global
Foo.classMethod // expect: global
