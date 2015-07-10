class Foo {
  bar=(value) { value }
}

var foo = Foo.new()
"a" + foo.bar = "value" // expect error
