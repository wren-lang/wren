var fiber = Fiber.new {
  System.print("fiber")
}

System.print("before") // expect: before
fiber.call()           // expect: fiber
System.print("after")  // expect: after
