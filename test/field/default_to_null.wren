class Foo {
  write { io.write(_field) }
}

(new Foo).write // expect: null
