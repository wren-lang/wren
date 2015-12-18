class Foo {
  def bar=(value) { value }
}

var foo = Foo.new()
a is foo.bar = "value" // expect error
