var fiber

fiber = Fiber.new {
  fiber.call(2) // expect runtime error: Fiber has already been called.
}

fiber.call(1)
