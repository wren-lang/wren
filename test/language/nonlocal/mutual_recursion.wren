class Foo {
  static bar { Bar.new() }
}

class Bar {
  static foo { Foo.new() }
}

IO.print(Foo.bar) // expect: instance of Bar
IO.print(Bar.foo) // expect: instance of Foo
