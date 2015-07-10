var fiber = Fiber.new {
  "s".unknown
}

fiber.try()
IO.print(fiber.isDone) // expect: true
