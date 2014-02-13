class Outer {
  method {
    IO.print(this) // expect: Outer

    fn {
      IO.print(this) // expect: Outer

      class Inner {
        method {
          IO.print(this) // expect: Inner
        }
        toString { return "Inner" }
      }

      (new Inner).method
    }.call
  }

  toString { return "Outer" }
}

(new Outer).method
