{
  var foo = "variable"

  class Foo {
    def construct new() {}

    def foo { "method" }

    def method {
      System.print(foo)
    }

    def static foo { "class method" }

    def static classMethod {
      System.print(foo)
    }
  }

  Foo.new().method // expect: method
  Foo.classMethod  // expect: class method
}
