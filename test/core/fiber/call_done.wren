var fiber = Fiber.new {
  System.print("call")
}

fiber.call() // expect: call
fiber.call() // expect runtime error: Cannot call a finished fiber.
