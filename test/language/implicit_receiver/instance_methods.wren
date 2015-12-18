class Foo {
  def construct new() {}

  def getter {
    System.print("getter")
  }

  def setter=(value) {
    System.print("setter")
  }

  def method(a) {
    System.print("method")
  }

  def test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.new().test

// TODO: Need to decide how these interact with globals.
