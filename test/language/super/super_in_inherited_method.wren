class A {
  def callSuperToString { super.toString }

  def toString { "A.toString" }
}

class B is A {
  def construct new() {}
}

System.print(B.new().callSuperToString) // expect: instance of B
