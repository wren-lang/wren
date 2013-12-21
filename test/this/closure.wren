class Foo {
  getClosure {
    return fn {
      return this.toString
    }
  }

  toString { return "Foo" }
}

var closure = (new Foo).getClosure
io.write(closure.call) // expect: Foo
