class Foo {
  def construct new() {}

  def bar=(value) {
    System.print("setter")
    return value
  }

  def test {
    bar = "value" // expect: setter

    {
      bar = "value" // expect: setter
      var bar = "local"
      bar = "value" // no expectation
    }

    bar = "value" // expect: setter
  }
}

Foo.new().test
