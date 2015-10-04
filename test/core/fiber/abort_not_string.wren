var fiber = Fiber.new {
  Fiber.abort(123)
}

System.print(fiber.try()) // expect: 123
System.print(fiber.isDone) // expect: true
System.print(fiber.error) // expect: 123
