class Foo {
  bar=(value) { value }
}

var foo = new Foo
a is foo.bar = "value" // expect error
