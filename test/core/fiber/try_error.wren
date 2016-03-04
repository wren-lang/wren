var fiber = Fiber.new {
  System.print("try")
  Fiber.abort("err")
}

fiber.try() // expect: try
fiber.try() // expect runtime error: Cannot try an aborted fiber.
