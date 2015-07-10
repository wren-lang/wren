class Outer {
  method {
    IO.print(this) // expect: Outer

    Fn.new {
      IO.print(this) // expect: Outer

      class Inner {
        method {
          IO.print(this) // expect: Inner
        }
        toString { "Inner" }
      }

      Inner.new().method
    }.call()
  }

  toString { "Outer" }
}

Outer.new().method
