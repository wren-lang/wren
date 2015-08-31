var fiber = Fiber.new {
  IO.print("fiber") // expect: fiber
}

IO.print(fiber.call()) // expect: null
