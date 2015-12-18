class Foo {
  construct new() {}
  def bar {}
}

System.print(Foo.new().bar) // expect: null
