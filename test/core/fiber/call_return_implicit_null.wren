var fiber = Fiber.new {
  IO.print("fiber")
}

var result = fiber.call() // expect: fiber
IO.print(result)          // expect: null
