class A {
  def callSuperToString {
    return Fn.new { super.toString }.call()
  }

  def toString { "A.toString" }
}

class B is A {
  construct new() {}
}

System.print(B.new().callSuperToString) // expect: instance of B
