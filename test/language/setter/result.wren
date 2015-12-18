class Foo {
  construct new() {}
  def bar=(value) { "result" }
}

var foo = Foo.new()
System.print(foo.bar = "value") // expect: result
