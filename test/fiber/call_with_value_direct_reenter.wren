var fiber

fiber = new Fiber {
  fiber.call(2) // expect runtime error: Fiber has already been called.
}

fiber.call(1)
