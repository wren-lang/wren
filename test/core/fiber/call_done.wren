var fiber = Fiber.new {
  System.print("call")
}

fiber() // expect: call
fiber() // expect runtime error: Cannot call a finished fiber.
