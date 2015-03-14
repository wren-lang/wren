var fiber = new Fiber {
  IO.print("fiber")
  return "result"
}

var result = fiber.call() // expect: fiber
IO.print(result)          // expect: result
