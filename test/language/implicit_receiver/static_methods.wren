class Foo {
  static getter {
    IO.print("getter")
  }

  static setter=(value) {
    IO.print("setter")
  }

  static method(a) {
    IO.print("method")
  }

  static test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.test
