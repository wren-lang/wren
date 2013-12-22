class Outer {
  method {
    IO.write(this.toString) // expect: Outer

    fn {
      IO.write(this.toString) // expect: Outer

      class Inner {
        method {
          IO.write(this.toString) // expect: Inner
        }
        toString { return "Inner" }
      }

      (new Inner).method
    }.call
  }

  toString { return "Outer" }
}

(new Outer).method
