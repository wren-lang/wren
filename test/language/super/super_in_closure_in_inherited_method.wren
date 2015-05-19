class A {
  callSuperToString {
    return new Fn { super.toString }.call()
  }

  toString { "A.toString" }
}

class B is A {}

IO.print((new B).callSuperToString) // expect: instance of B
