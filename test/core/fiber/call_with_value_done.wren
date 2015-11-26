var fiber = Fiber.new {
  System.print("call")
}

fiber(1) // expect: call
fiber(2) // expect runtime error: Cannot call a finished fiber.
