class Foo {
  bar = value { IO.write("set") }
  bar { IO.write("get") }
}

var foo = new Foo
foo.bar = "value" // expect: set
foo.bar // expect: get
