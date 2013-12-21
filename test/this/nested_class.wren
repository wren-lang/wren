class Outer {
  method {
    io.write(this.toString) // expect: Outer

    fn {
      io.write(this.toString) // expect: Outer

      class Inner {
        method {
          io.write(this.toString) // expect: Inner
        }
        toString { return "Inner" }
      }

      (new Inner).method
    }.call
  }

  toString { return "Outer" }
}

(new Outer).method
