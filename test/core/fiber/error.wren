var fiber = Fiber.new {
  "s".unknown
}

System.print(fiber.error) // expect: null
System.print(fiber.try()) // expect: String does not implement 'unknown'.
System.print(fiber.error) // expect: String does not implement 'unknown'.
