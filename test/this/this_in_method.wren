class Foo {
  bar { return this }
  baz { return "baz" }
}

IO.print((new Foo).bar.baz) // expect: baz
