var fiber = Fiber.create {
  IO.print("fiber")
}

// The first value passed to the fiber is ignored, since there's no yield call
// to return it.
IO.print("before")   // expect: before
fiber.run("ignored") // expect: fiber
IO.print("after")    // expect: after
