var fiber = Fiber.new {|v|
  System.print("before")
  System.print(v)
  v = Fiber.yield()
  System.print(v)
  true.unknownMethod
  System.print("after")
}

fiber.try("value1")
// expect: before
// expect: value1
System.print(fiber.try("value2"))
// expect: value2
// expect: Bool does not implement 'unknownMethod'.
System.print("after try") // expect: after try
