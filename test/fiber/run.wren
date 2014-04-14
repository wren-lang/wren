var fiber = new Fiber {
  IO.print("fiber")
}

IO.print("before") // expect: before
fiber.run          // expect: fiber

// This does not get run since we exit when the run fiber completes.
IO.print("nope")
