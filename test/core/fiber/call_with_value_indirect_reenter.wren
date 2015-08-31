var A = Fiber.new {
  B.call(3) // expect runtime error: Fiber has already been called.
}

var B = Fiber.new {
  A.call(2)
}

B.call(1)
