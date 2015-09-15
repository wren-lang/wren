var fiber = Fiber.new {
  System.print("fiber") // expect: fiber
}

System.print(fiber.call()) // expect: null
