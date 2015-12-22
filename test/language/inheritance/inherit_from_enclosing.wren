class Outer {
  construct new() {}

  def outerBefore() { System.print("Outer before") }
  def shadowOuter() { System.print("bad") }

  class Inner {
    construct new() {}

    def innerBefore() { System.print("Inner before") }
    def shadowInner() { System.print("bad") }

    class MoreInner {
      construct new() {}

      def shadowOuter() { System.print("ok") }
      def shadowInner() { System.print("ok") }
    }

    def innerAfter() { System.print("Inner after") }
  }

  def outerAfter() { System.print("Outer after") }
}

var moreInner = Outer.new().Inner.new().MoreInner.new()
moreInner.outerBefore() // expect: Outer before
moreInner.outerAfter()  // expect: Outer after
moreInner.innerBefore() // expect: Inner before
moreInner.innerAfter()  // expect: Inner after
moreInner.shadowOuter() // expect: ok
moreInner.shadowInner() // expect: ok

// TODO: Test how super interacts with enclosing classes.
