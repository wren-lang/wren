var fiber = Fiber.new {
  "s".unknown
}

IO.print(fiber.error) // expect: null
IO.print(fiber.try()) // expect: String does not implement 'unknown'.
IO.print(fiber.error) // expect: String does not implement 'unknown'.
