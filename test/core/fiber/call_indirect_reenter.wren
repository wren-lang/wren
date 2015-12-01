var a
var b

a = Fiber.new {
  b() // expect runtime error: Fiber has already been called.
}

b = Fiber.new {
  a()
}

b()
