class Foo {
  construct new() {}

  getter {
    System.print("getter")
  }

  setter=(value) {
    System.print("setter")
  }

  method(a) {
    System.print("method")
  }
}

class Bar is Foo {
  construct new() {}

  test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Bar.new().test
