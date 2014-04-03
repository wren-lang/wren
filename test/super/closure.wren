class Base {
  toString { "Base" }
}

class Derived is Base {
  getClosure { new Fn { super.toString } }
  toString { "Derived" }
}

var closure = (new Derived).getClosure
IO.print(closure.call) // expect: Base
