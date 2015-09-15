class Foo {
  static getter {
    System.print("getter")
  }

  static setter=(value) {
    System.print("setter")
  }

  static method(a) {
    System.print("method")
  }

  static test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.test
