class Foo {
  def construct new() {}
  def bar { this }
  def baz { "baz" }
}

System.print(Foo.new().bar.baz) // expect: baz
