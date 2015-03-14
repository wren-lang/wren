var fiber = new Fiber {
  IO.print("fiber")
}

var result = fiber.call() // expect: fiber
IO.print(result)          // expect: null
