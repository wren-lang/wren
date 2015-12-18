var foo = "variable"

class Foo {
  construct new() {}

  def foo { "method" }

  def method {
    System.print(foo)
  }

  static def foo { "class method" }

  static def classMethod {
    System.print(foo)
  }
}

Foo.new().method // expect: method
Foo.classMethod  // expect: class method
