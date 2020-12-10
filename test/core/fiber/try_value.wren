var fiber = Fiber.new {|v|
  System.print("before")
  System.print(v)
  true.unknownMethod
  System.print("after")
}

System.print(fiber.try("value"))
// expect: before
// expect: value
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try
