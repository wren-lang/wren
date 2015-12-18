class Outer {
  def construct new() {}

  def method {
    System.print(this) // expect: Outer

    Fn.new {
      System.print(this) // expect: Outer

      class Inner {
        def construct new() {}

        def method {
          System.print(this) // expect: Inner
        }

        def toString { "Inner" }
      }

      Inner.new().method
    }.call()
  }

  def toString { "Outer" }
}

Outer.new().method
