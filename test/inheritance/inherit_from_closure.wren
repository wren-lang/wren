var ClosureType

{
  var a = "a"
  ClosureType = new Fn { IO.print(a) }.type
}

class Subclass is ClosureType {} // expect runtime error: Subclass cannot inherit from Fn.
