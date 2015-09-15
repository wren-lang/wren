class Foo {
  construct new() {}
  write { System.print(_field) } // Compile a use of the field...
  init { _field = "value" }  // ...before an assignment to it.
}

var foo = Foo.new()
// But invoke them in the right order.
foo.init
foo.write // expect: value
