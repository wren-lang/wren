var fiber

fiber = Fiber.new {
  fiber() // expect runtime error: Fiber has already been called.
}

fiber()
