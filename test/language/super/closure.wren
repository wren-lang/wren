class Base {
  toString { "Base" }
}

class Derived is Base {
  construct new() {}
  getClosure { fn () { super.toString } }
  toString { "Derived" }
}

var closure = Derived.new().getClosure
System.print(closure()) // expect: Base
