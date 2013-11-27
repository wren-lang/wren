class Foo {
  write { io.write(_field) }
}

Foo.new.write // expect: null
