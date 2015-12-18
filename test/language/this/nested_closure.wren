class Foo {
  construct new() {}
  def getClosure { Fn.new { Fn.new { Fn.new { toString } } } }
  def toString { "Foo" }
}

var closure = Foo.new().getClosure
System.print(closure.call().call().call()) // expect: Foo
