class Foo {
  construct new() {}
  write { IO.print(_field) }
}

Foo.new().write // expect: null
