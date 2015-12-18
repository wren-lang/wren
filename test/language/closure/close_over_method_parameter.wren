var F = null

class Foo {
  def construct new() {}

  def method(param) {
    F = Fn.new {
      System.print(param)
    }
  }
}

Foo.new().method("param")
F.call() // expect: param
