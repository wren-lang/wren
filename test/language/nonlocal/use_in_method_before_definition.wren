class Foo {
  def construct new() {}

  def method {
    System.print(Global)
  }

  def static classMethod {
    System.print(Global)
  }
}

var Global = "global"

Foo.new().method // expect: global
Foo.classMethod // expect: global
