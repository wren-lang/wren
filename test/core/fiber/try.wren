var fiber = Fiber.new {
  System.print("before")
  true.unknownMethod
  System.print("after")
}

System.print(fiber.try())
// expect: before
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try
