var a
var b

a = Fiber.new {
  b.call() // expect runtime error: Fiber has already been called.
}

b = Fiber.new {
  a.call()
}

b.call()
