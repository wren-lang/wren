class Foo {
  def bar=(value) { value }
}

var foo = Foo.new()
"a" + foo.bar = "value" // expect error
