class Foo {
  static write { System.print(__field) } // Compile a use of the field...
  static init { __field = "value" }  // ...before an assignment to it.
}

// But invoke them in the right order.
Foo.init
Foo.write // expect: value
