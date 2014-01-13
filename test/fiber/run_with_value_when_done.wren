var fiber = Fiber.create(fn {
  IO.print("run")
})

fiber.run(1) // expect: run
fiber.run(2) // expect runtime error: Cannot run a finished fiber.
