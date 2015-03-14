class Foo {
  getClosure { new Fn { new Fn { new Fn { toString } } } }
  toString { "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call().call().call()) // expect: Foo
