var fiber = Fiber.new {
  System.print("fiber") // expect: fiber
}

System.print(fiber()) // expect: null
