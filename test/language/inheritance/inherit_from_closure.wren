var ClosureType

{
  var a = "a"
  ClosureType = Fn.new { IO.print(a) }.type
}

class Subclass is ClosureType {} // expect runtime error: Class 'Subclass' cannot inherit from built-in class 'Fn'.
