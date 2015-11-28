class Foo {
  construct new() {}
  getClosure { fn () { fn () { fn () { toString } } } }
  toString { "Foo" }
}

var closure = Foo.new().getClosure
System.print(closure()()()) // expect: Foo
