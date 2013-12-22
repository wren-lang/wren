class Foo {
  getClosure {
    return fn {
      return this.toString
    }
  }

  toString { return "Foo" }
}

var closure = (new Foo).getClosure
IO.write(closure.call) // expect: Foo
