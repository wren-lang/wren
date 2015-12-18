class Foo {
  def construct new() {}
  def bar {}
}

System.print(Foo.new().bar) // expect: null
