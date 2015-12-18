class Foo {
  construct new() {}
  def bar=(value) { System.print("set") }
  def bar { System.print("get") }
}

var foo = Foo.new()
foo.bar = "value" // expect: set
foo.bar // expect: get
