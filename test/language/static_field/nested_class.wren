class Outer {
  construct new() {}

  static def staticMethod {
    __field = "outer"
    System.print(__field) // expect: outer

    class Inner {
      construct new() {}

      static def staticMethod {
        __field = "inner"
        System.print(__field) // expect: inner
      }
    }

    Inner.staticMethod
    System.print(__field) // expect: outer
  }

  def instanceMethod {
    __field = "outer"
    System.print(__field) // expect: outer

    class Inner {
      construct new() {}

      def instanceMethod {
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
