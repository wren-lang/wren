class A {
  callSuperToString { super.toString }

  toString { "A.toString" }
}

class B is A {}

IO.print((new B).callSuperToString) // expect: instance of B
