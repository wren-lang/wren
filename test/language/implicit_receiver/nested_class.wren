class Outer {
  def construct new() {}

  def getter {
    System.print("outer getter")
  }

  def setter=(value) {
    System.print("outer setter")
  }

  def method(a) {
    System.print("outer method")
  }

  def test {
    getter            // expect: outer getter
    setter = "value"  // expect: outer setter
    method("arg")     // expect: outer method

    class Inner {
      def construct new() {}

      def getter {
        System.print("inner getter")
      }

      def setter=(value) {
        System.print("inner setter")
      }

      def method(a) {
        System.print("inner method")
      }

      def test {
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
