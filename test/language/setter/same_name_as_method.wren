class Foo {
  construct new() {}
  bar=(value) { IO.print("set") }
  bar { IO.print("get") }
}

var foo = Foo.new()
foo.bar = "value" // expect: set
foo.bar // expect: get
