class Foo {
  construct new() {}
  bar { this }
  baz { "baz" }
}

IO.print(Foo.new().bar.baz) // expect: baz
