class Foo {
  construct new() {}

  def method {
    System.print(Global)
  }

  static def classMethod {
    System.print(Global)
  }
}

var Global = "global"

Foo.new().method // expect: global
Foo.classMethod // expect: global
