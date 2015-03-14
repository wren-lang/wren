var fiber = new Fiber {
  Fiber.abort("Error message.")
}

IO.print(fiber.try()) // expect: Error message.
IO.print(fiber.isDone) // expect: true
IO.print(fiber.error) // expect: Error message.
