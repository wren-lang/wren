var fiber

fiber = Fiber.new {
  fiber(2) // expect runtime error: Fiber has already been called.
}

fiber(1)
