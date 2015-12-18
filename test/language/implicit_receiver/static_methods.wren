class Foo {
  def static getter {
    System.print("getter")
  }

  def static setter=(value) {
    System.print("setter")
  }

  def static method(a) {
    System.print("method")
  }

  def static test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.test
