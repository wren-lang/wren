var fiber

fiber = Fiber.new {
  fiber.call() // expect runtime error: Fiber has already been called.
}

fiber.call()
