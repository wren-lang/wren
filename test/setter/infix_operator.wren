class Foo {
  bar=(value) { value }
}

var foo = new Foo
"a" + foo.bar = "value" // expect error
