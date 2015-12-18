class Foo {
  def construct new() {}

  def method {
    return "ok"
    System.print("bad")
  }
}

System.print(Foo.new().method) // expect: ok
