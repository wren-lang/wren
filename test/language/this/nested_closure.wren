class Foo {
  getClosure { Fn.new { Fn.new { Fn.new { toString } } } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
IO.print(closure.call().call().call()) // expect: Foo
