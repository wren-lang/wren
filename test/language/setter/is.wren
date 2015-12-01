class Foo {
  bar=(value) { value }
}

var foo = Foo.new()
foo is foo.bar = "value" // expect error
