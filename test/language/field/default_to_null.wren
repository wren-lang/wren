class Foo {
  construct new() {}
  write { System.print(_field) }
}

Foo.new().write // expect: null
