var fiber = Fiber.new {
  IO.print("fiber")
  return "result" // expect: fiber
}

IO.print(fiber.call()) // expect: result
