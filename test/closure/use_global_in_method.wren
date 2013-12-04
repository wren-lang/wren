var global = "global"
// TODO(bob): Forward reference to global declared after use.

class Foo {
  method {
    io.write(global)
  }

  static classMethod {
    io.write(global)
  }
}

Foo.new.method // expect: global
Foo.classMethod // expect: global
