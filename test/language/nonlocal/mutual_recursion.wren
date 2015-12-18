class Foo {
  construct new() {}
  static def bar { Bar.new() }
}

class Bar {
  construct new() {}
  static def foo { Foo.new() }
}

System.print(Foo.bar) // expect: instance of Bar
System.print(Bar.foo) // expect: instance of Foo
