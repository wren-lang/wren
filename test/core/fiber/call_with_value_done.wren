var fiber = Fiber.new {
  System.print("call")
}

fiber.call(1) // expect: call
fiber.call(2) // expect runtime error: Cannot call a finished fiber.
