class Foo {
  bar { return this }
  baz { return "baz" }
}

IO.print((new Foo).bar.baz) // expect: baz

// TODO: Test that "this" is the class when in a static method.
//       (Or disallow "this" in statics? It's useful since statics are
//       inherited.)
