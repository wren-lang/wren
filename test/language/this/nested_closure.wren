class Foo {
  construct new() {}
  getClosure { Fn.new { Fn.new { Fn.new { toString } } } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
System.print(closure.call().call().call()) // expect: Foo
