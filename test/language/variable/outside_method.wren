var foo = "variable"

class Foo {
  construct new() {}

  foo { "method" }

  method {
    IO.print(foo)
  }

  static foo { "class method" }

  static classMethod {
    IO.print(foo)
  }
}

Foo.new().method // expect: method
Foo.classMethod  // expect: class method
