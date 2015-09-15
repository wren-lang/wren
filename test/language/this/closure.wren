class Foo {
  construct new() {}
  getClosure { Fn.new { toString } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
System.print(closure.call()) // expect: Foo
