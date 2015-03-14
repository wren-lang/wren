var fiber = new Fiber {
  IO.print("fiber")
}

// The first value passed to the fiber is ignored, since there's no yield call
// to return it.
IO.print("before")    // expect: before
fiber.run("ignored") // expect: fiber

// This does not get run since we exit when the run fiber completes.
IO.print("nope")
