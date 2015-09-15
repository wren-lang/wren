var fiber = Fiber.new {
  System.print("fiber")
  return "result" // expect: fiber
}

System.print(fiber.call()) // expect: result
