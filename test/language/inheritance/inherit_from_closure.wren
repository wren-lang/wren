var ClosureType

{
  var a = "a"
  ClosureType = fn () { System.print(a) }.type
}

class Subclass is ClosureType {} // expect runtime error: Class 'Subclass' cannot inherit from built-in class 'Fn'.
