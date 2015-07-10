var fiber = Fiber.new {
  IO.print("fiber")
  var result = Fiber.yield()
  IO.print(result)
}

fiber.call() // expect: fiber
IO.print("main") // expect: main
fiber.run("run") // expect: run

// This does not get run since we exit when the run fiber completes.
IO.print("nope")
