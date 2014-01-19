class Foo {
  static write { IO.print(__field) }
}

Foo.write // expect: null
