class Outer {
  construct new() {}

  method {
    System.print(this) // expect: Outer

    Fn.new {
      System.print(this) // expect: Outer

      class Inner {
        construct new() {}

        method {
          System.print(this) // expect: Inner
        }
        toString { "Inner" }
      }

      Inner.new().method
    }.call()
  }

  toString { "Outer" }
}

Outer.new().method
