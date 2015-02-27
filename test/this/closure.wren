class Foo {
  getClosure { new Fn { toString } }
  toString { "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call()) // expect: Foo
