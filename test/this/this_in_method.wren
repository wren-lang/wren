class Foo {
  bar { return this }
  baz { return "baz" }
}

IO.write((new Foo).bar.baz) // expect: baz
