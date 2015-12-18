class Foo {
  def construct new() {}

  def bar=(value) {
    System.print(value)
  }
}

var foo = Foo.new()
foo.bar = "value" // expect: value
