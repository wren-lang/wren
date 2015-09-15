class Foo {
  static write { System.print(__field) }
}

Foo.write // expect: null
