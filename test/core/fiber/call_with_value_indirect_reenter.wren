var a = Fiber.new {
  b(3) // expect runtime error: Fiber has already been called.
}

var b = Fiber.new {
  a(2)
}

b(1)
