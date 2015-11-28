class Outer {
  construct new() {}

  method {
    System.print(this) // expect: Outer

    fn () {
      System.print(this) // expect: Outer

      class Inner {
        construct new() {}

        method {
          System.print(this) // expect: Inner
        }
        toString { "Inner" }
      }

      Inner.new().method
    }()
  }

  toString { "Outer" }
}

Outer.new().method
