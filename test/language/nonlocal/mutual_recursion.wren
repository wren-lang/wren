class Foo {
  def construct new() {}
  def static bar { Bar.new() }
}

class Bar {
  def construct new() {}
  def static foo { Foo.new() }
}

System.print(Foo.bar) // expect: instance of Bar
System.print(Bar.foo) // expect: instance of Foo
