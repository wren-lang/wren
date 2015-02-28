var fiber = new Fiber {
  IO.print("call")
}

fiber.call() // expect: call
fiber.call() // expect runtime error: Cannot call a finished fiber.
