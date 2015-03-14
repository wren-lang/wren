class Foo {
  write { IO.print(_field) }
}

(new Foo).write // expect: null
