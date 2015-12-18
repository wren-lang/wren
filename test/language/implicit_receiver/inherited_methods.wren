class Foo {
  construct new() {}

  def getter {
    System.print("getter")
  }

  def setter=(value) {
    System.print("setter")
  }

  def method(a) {
    System.print("method")
  }
}

class Bar is Foo {
  construct new() {}

  def test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Bar.new().test
