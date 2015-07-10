var a
var b

a = Fiber.new {
  b.call(3) // expect runtime error: Fiber has already been called.
}

b = Fiber.new {
  a.call(2)
}

b.call(1)
