var fiber = new Fiber {
  "s".unknown
}

fiber.try()
IO.print(fiber.isDone) // expect: true
