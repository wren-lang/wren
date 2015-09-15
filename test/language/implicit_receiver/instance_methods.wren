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

  test {
    getter            // expect: getter
    setter = "value"  // expect: setter
    method("arg")     // expect: method
  }
}

Foo.new().test

// TODO: Need to decide how these interact with globals.
