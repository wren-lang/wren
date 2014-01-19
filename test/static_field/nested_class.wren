class Outer {
  static staticMethod {
    __field = "outer"
    IO.print(__field) // expect: outer

    class Inner {
      static staticMethod {
        __field = "inner"
        IO.print(__field) // expect: inner
      }
    }

    Inner.staticMethod
    IO.print(__field) // expect: outer
  }

  instanceMethod {
    __field = "outer"
    IO.print(__field) // expect: outer

    class Inner {
      instanceMethod {
        __field = "inner"
        IO.print(__field) // expect: inner
      }
    }

    (new Inner).instanceMethod
    IO.print(__field) // expect: outer
  }
}

Outer.staticMethod
(new Outer).instanceMethod
