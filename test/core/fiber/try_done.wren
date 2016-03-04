var fiber = Fiber.new {
  System.print("try")
}

fiber.try() // expect: try
fiber.try() // expect runtime error: Cannot try a finished fiber.
