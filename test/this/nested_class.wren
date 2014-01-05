class Outer {
  method {
    IO.print(this.toString) // expect: Outer

    fn {
      IO.print(this.toString) // expect: Outer

      class Inner {
        method {
          IO.print(this.toString) // expect: Inner
        }
        toString { return "Inner" }
      }

      (new Inner).method
    }.call
  }

  toString { return "Outer" }
}

(new Outer).method
