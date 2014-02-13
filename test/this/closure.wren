class Foo {
  getClosure {
    return fn {
      return toString
    }
  }

  toString { return "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call) // expect: Foo
