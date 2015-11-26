var A = Fiber.new {
  B(3) // expect runtime error: Fiber has already been called.
}

var B = Fiber.new {
  A(2)
}

B(1)
