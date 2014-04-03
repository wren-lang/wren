class Foo {
  getClosure {
    return new Fn {
      return new Fn {
        return new Fn {
          return toString
        }
      }
    }
  }

  toString { return "Foo" }
}

var closure = (new Foo).getClosure
IO.print(closure.call.call.call) // expect: Foo
