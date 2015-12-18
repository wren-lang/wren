class Foo {
  def bar=(value) { value }
}

var foo = Foo.new()
(foo.bar) = "value" // expect error
