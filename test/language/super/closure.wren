class Base {
  def toString { "Base" }
}

class Derived is Base {
  construct new() {}
  def getClosure { Fn.new { super.toString } }
  def toString { "Derived" }
}

var closure = Derived.new().getClosure
System.print(closure.call()) // expect: Base
