var fiber = new Fiber {
  IO.print("run")
}

fiber.run // expect: run
fiber.run // expect runtime error: Cannot run a finished fiber.
