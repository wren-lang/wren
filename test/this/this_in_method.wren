class Foo {
  bar { return this }
  baz { return "baz" }
}

io.write((new Foo).bar.baz) // expect: baz
