class Foo {
  write { IO.print(_field) }
}

Foo.new().write // expect: null
