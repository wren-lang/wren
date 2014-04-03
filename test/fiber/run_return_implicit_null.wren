var fiber = Fiber.create {
  IO.print("fiber")
}

var result = fiber.run // expect: fiber
IO.print(result)       // expect: null
