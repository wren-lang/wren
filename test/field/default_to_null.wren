class Foo {
  write { IO.write(_field) }
}

(new Foo).write // expect: null
