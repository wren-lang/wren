class Foo {
  static bar { new Bar }
}

class Bar {
  static foo { new Foo }
}

IO.print(Foo.bar) // expect: instance of Bar
IO.print(Bar.foo) // expect: instance of Foo
