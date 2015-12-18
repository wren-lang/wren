class Foo {
  static def write { System.print(__field) }
}

Foo.write // expect: null
