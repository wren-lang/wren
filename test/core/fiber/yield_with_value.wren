var fiber = Fiber.new {
  System.print("fiber 1")
  Fiber.yield("yield 1")
  System.print("fiber 2")
  Fiber.yield("yield 2")
  System.print("fiber 3")
}

System.print(fiber.call())  // expect: fiber 1
                            // expect: yield 1
System.print(fiber.call())  // expect: fiber 2
                            // expect: yield 2
System.print(fiber.call())  // expect: fiber 3
                            // expect: null
