class Outer {
  getter {
    IO.print("outer getter")
  }

  setter=(value) {
    IO.print("outer setter")
  }

  method(a) {
    IO.print("outer method")
  }

  test {
    getter            // expect: outer getter
    setter = "value"  // expect: outer setter
    method("arg")     // expect: outer method

    class Inner {
      getter {
        IO.print("inner getter")
      }

      setter=(value) {
        IO.print("inner setter")
      }

      method(a) {
        IO.print("inner method")
      }

      test {
        getter            // expect: inner getter
        setter = "value"  // expect: inner setter
        method("arg")     // expect: inner method
      }
    }

    (new Inner).test

    getter            // expect: outer getter
    setter = "value"  // expect: outer setter
    method("arg")     // expect: outer method
  }
}

(new Outer).test
