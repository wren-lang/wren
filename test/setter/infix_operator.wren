class Foo {
  bar = value { return value }
}

var foo = new Foo
"a" + foo.bar = "value" // expect error
