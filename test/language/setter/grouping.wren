class Foo {
  bar=(value) { value }
}

var foo = Foo.new()
(foo.bar) = "value" // expect error
