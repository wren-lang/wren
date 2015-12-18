class Foo {
  static def getter {
    System.print("getter")
  }

  static def setter=(value) {
    System.print("setter")
  }

  static def method(a) {
    System.print("method")
  }

  static def test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.test
