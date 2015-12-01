var foo = "variable"

class Foo {
  construct new() {}

  foo { "method" }

  test() {
    System.print(foo)
  }

  static foo { "class method" }

  static test() {
    System.print(foo)
  }
}

Foo.new().test() // expect: variable
Foo.test()       // expect: variable
