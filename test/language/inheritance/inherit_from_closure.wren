var ClosureType

{
  var a = "a"
  ClosureType = Fn.new { IO.print(a) }.type
}

class Subclass is ClosureType {} // expect runtime error: Subclass cannot inherit from Fn.
