var fiber = Fiber.new {
  System.print("fiber")
}

System.print("before") // expect: before
fiber.call("value")    // expect: fiber
System.print("after")  // expect: after
