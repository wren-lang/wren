var fiber

fiber = new Fiber {
  fiber.try() // expect runtime error: Fiber has already been called.
}

fiber.call()
