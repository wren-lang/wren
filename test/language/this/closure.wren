class Foo {
  construct new() {}
  def getClosure { Fn.new { toString } }
  def toString { "Foo" }
}

var closure = Foo.new().getClosure
System.print(closure.call()) // expect: Foo
