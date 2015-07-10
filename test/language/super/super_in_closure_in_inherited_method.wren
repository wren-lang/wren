class A {
  callSuperToString {
    return Fn.new { super.toString }.call()
  }

  toString { "A.toString" }
}

class B is A {}

IO.print(B.new().callSuperToString) // expect: instance of B
