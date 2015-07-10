var fiber

fiber = Fiber.new {
  fiber.try() // expect runtime error: Fiber has already been called.
}

fiber.call()
