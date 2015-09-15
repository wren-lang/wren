class Outer {
  construct new() {}

  getter {
    System.print("outer getter")
  }

  setter=(value) {
    System.print("outer setter")
  }

  method(a) {
    System.print("outer method")
  }

  test {
    getter            // expect: outer getter
    setter = "value"  // expect: outer setter
    method("arg")     // expect: outer method

    class Inner {
      construct new() {}

      getter {
        System.print("inner getter")
      }

      setter=(value) {
        System.print("inner setter")
      }

      method(a) {
        System.print("inner method")
      }

      test {
        getter            // expect: inner getter
        setter = "value"  // expect: inner setter
        method("arg")     // expect: inner method
      }
    }

    Inner.new().test

    getter            // expect: outer getter
    setter = "value"  // expect: outer setter
    method("arg")     // expect: outer method
  }
}

Outer.new().test
