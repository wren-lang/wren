class A {
  callSuperToString { super.toString }

  toString { "A.toString" }
}

class B is A {}

IO.print(B.new().callSuperToString) // expect: instance of B
