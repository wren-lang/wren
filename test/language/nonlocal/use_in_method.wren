var Global = "global"

class Foo {
  def construct new() {}

  def method {
    System.print(Global)
  }

  def static classMethod {
    System.print(Global)
  }
}

Foo.new().method // expect: global
Foo.classMethod // expect: global
