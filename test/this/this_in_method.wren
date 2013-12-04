class Foo {
  bar { return this }
  baz { return "baz" }
}

io.write(Foo.new.bar.baz) // expect: baz
