class Foo {
  bar = value { return value }
}

var foo = new Foo
a is foo.bar = "value" // expect error
