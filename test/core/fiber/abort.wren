var fiber = Fiber.new {
  Fiber.abort("Error message.")
}

System.print(fiber.try()) // expect: Error message.
System.print(fiber.isDone) // expect: true
System.print(fiber.error) // expect: Error message.
