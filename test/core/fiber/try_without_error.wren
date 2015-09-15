var fiber = Fiber.new {
  System.print("fiber")
}

System.print("before")    // expect: before
System.print(fiber.try()) // expect: fiber
                      // expect: null
System.print("after")     // expect: after
