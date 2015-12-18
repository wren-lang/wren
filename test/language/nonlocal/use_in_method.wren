var Global = "global"

class Foo {
  construct new() {}

  def method {
    System.print(Global)
  }

  static def classMethod {
    System.print(Global)
  }
}

Foo.new().method // expect: global
Foo.classMethod // expect: global
