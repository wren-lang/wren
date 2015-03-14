class Foo {
  bar { this }
  baz { "baz" }
}

IO.print((new Foo).bar.baz) // expect: baz
