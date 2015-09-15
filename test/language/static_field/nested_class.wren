class Outer {
  construct new() {}

  static staticMethod {
    __field = "outer"
    System.print(__field) // expect: outer

    class Inner {
      construct new() {}

      static staticMethod {
        __field = "inner"
        System.print(__field) // expect: inner
      }
    }

    Inner.staticMethod
    System.print(__field) // expect: outer
  }

  instanceMethod {
    __field = "outer"
    System.print(__field) // expect: outer

    class Inner {
      construct new() {}

      instanceMethod {
        __field = "inner"
        System.print(__field) // expect: inner
      }
    }

    Inner.new().instanceMethod
    System.print(__field) // expect: outer
  }
}

Outer.staticMethod
Outer.new().instanceMethod
