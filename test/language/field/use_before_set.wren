class Foo {
  write { IO.print(_field) } // Compile a use of the field...
  init { _field = "value" }  // ...before an assignment to it.
}

var foo = new Foo
// But invoke them in the right order.
foo.init
foo.write // expect: value
