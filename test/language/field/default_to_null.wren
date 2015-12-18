class Foo {
  def construct new() {}
  def write { System.print(_field) }
}

Foo.new().write // expect: null
