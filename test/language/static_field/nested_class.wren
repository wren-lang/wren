class Outer {
  def construct new() {}

  def static staticMethod {
    __field = "outer"
    System.print(__field) // expect: outer

    class Inner {
      def construct new() {}

      def static staticMethod {
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
      def construct new() {}

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
