class Foo {
  construct new() {}
  bar=(value) { System.print("set") }
  bar { System.print("get") }
}

var foo = Foo.new()
foo.bar = "value" // expect: set
foo.bar // expect: get
