var fiber = Fiber.new {
  Fiber.abort("Error!")
  System.print("should not get here")
}

fiber.try()
fiber("value") // expect runtime error: Cannot call an aborted fiber.
