var fiber = new Fiber {
  "s".unknown
}

IO.print(fiber.error) // expect: null
IO.print(fiber.try)   // expect: String does not implement method 'unknown' with 0 arguments.
IO.print(fiber.error) // expect: String does not implement method 'unknown' with 0 arguments.
