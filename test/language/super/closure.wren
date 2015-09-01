class Base {
  toString { "Base" }
}

class Derived is Base {
  construct new() {}
  getClosure { Fn.new { super.toString } }
  toString { "Derived" }
}

var closure = Derived.new().getClosure
IO.print(closure.call()) // expect: Base
