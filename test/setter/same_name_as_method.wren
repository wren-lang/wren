class Foo {
  bar = value { io.write("set") }
  bar { io.write("get") }
}

var foo = new Foo
foo.bar = "value" // expect: set
foo.bar // expect: get
