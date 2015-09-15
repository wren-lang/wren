var fiber = Fiber.new {
  System.print("fiber 1")
  Fiber.yield()
  System.print("fiber 2")
  Fiber.yield()
  System.print("fiber 3")
}

fiber.call()          // expect: fiber 1
System.print("main 1")    // expect: main 1
fiber.call()          // expect: fiber 2
System.print("main 2")    // expect: main 2
fiber.call()          // expect: fiber 3
System.print("main 3")    // expect: main 3
