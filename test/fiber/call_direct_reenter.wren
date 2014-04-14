var fiber

fiber = new Fiber {
  fiber.call // expect runtime error: Fiber has already been called.
}

fiber.call
