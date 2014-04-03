var fiber = Fiber.create {
  IO.print("run")
}

fiber.run(1) // expect: run
fiber.run(2) // expect runtime error: Cannot run a finished fiber.
