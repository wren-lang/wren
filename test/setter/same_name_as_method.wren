class Foo {
  bar = value { IO.print("set") }
  bar { IO.print("get") }
}

var foo = new Foo
foo.bar = "value" // expect: set
foo.bar // expect: get
