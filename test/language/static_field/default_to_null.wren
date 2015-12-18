class Foo {
  def static write { System.print(__field) }
}

Foo.write // expect: null
