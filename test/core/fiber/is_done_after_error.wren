var fiber = Fiber.new {
  "s".unknown
}

fiber.try()
System.print(fiber.isDone) // expect: true
