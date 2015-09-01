class Foo {
  construct new() {}

  getter {
    IO.print("getter")
  }

  setter=(value) {
    IO.print("setter")
  }

  method(a) {
    IO.print("method")
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
