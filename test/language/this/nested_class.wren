class Outer {
  construct new() {}

  def method {
    System.print(this) // expect: Outer

    Fn.new {
      System.print(this) // expect: Outer

      class Inner {
        construct new() {}

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
